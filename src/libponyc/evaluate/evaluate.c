#include "evaluate.h"
#include "builtin/method_table.h"
#include "../ast/astbuild.h"
#include "../ast/lexer.h"
#include "../ast/printbuf.h"
#include "../codegen/codegen.h"
#include "../expr/literal.h"
#include "../type/alias.h"
#include "../type/assemble.h"
#include "../type/lookup.h"
#include "../type/reify.h"
#include "../type/subtype.h"
#include "../pass/expr.h"
#include "../pkg/package.h"
#include "ponyassert.h"

void evaluate_init(pass_opt_t* opt)
{
  if(opt->limit >= PASS_EVALUATE)
    methodtab_init();
}

void evaluate_done(pass_opt_t* opt)
{
  if(opt->limit >= PASS_EVALUATE)
    methodtab_done();
}

bool expr_constant(pass_opt_t* opt, ast_t* ast)
{
  pony_assert(ast_id(ast) == TK_CONSTANT);

  ast_t* expression = ast_child(ast);
  ast_settype(ast, control_type_add_branch(opt, NULL, expression));
  ast_t* type = ast_type(ast);
  if(is_typecheck_error(type))
    return false;

  // The '#' denotes that the expression is a compile-time expression, as such
  // it really must be val. We can only read values generated by the compiler.
  if(!is_type_literal(type))
  {
    // TODO: not sure whether we need to even check whether we can recover the
    // the type so much as force it to be a TK_VAL
    ast_t* val_type = recover_type(type, TK_VAL);
    if(val_type == NULL)
    {
      ast_error(opt->check.errors, expression,
        "cannot recover compile-time object to val capability");
      return false;
    }

    ast_settype(expression, val_type);
    ast_free_unattached(type);
    ast_settype(ast, control_type_add_branch(opt, NULL, expression));
  }

  return true;
}

static bool evaluate(pass_opt_t* opt, ast_t* this, ast_t* expression,
  ast_t** result);

/*
 * Construct a mapping from the given lhs identifier to the rhs value. This
 * method assumes that all mappings are valid, the method is provided as a
 * convenience to extract the identifier name.
 */
static bool assign_value(pass_opt_t* opt, ast_t* this, ast_t* left,
  ast_t* right, ast_t** result)
{
  (void) opt;
  pony_assert(left != NULL);
  pony_assert(right != NULL);

  switch(ast_id(left))
  {
    case TK_VAR:
    case TK_VARREF:
    case TK_LET:
    case TK_FLET:
    case TK_PARAM:
      return ast_setvalue(left, ast_name(ast_child(left)), right, result);

    case TK_FLETREF:
    case TK_FVARREF:
    {
      AST_GET_CHILDREN(left, receiver, id);

      ast_t* evaluated_receiver;
      evaluate(opt, this, receiver, &evaluated_receiver);
      pony_assert(evaluated_receiver != NULL);

      return ast_setvalue(evaluated_receiver, ast_name(id), right, result);
    }

    default:
      pony_assert(0);
      return false;
  }
}

// Bind a value so that it can be used in later compile-time expressions
static bool bind_value(pass_opt_t* opt, ast_t* left, ast_t* right,
  ast_t** result)
{
  (void) opt;
  pony_assert(left != NULL);
  pony_assert(right != NULL);

  token_id left_id = ast_id(left);
  if(left_id == TK_VAR || left_id == TK_VARREF ||
     left_id == TK_FVAR || left_id == TK_FVARREF)
    return false;

  return assign_value(opt, NULL, left, right, result);
}

static void construct_object_hygienic_name(printbuf_t* buf, pass_opt_t* opt,
  ast_t* type)
{
  switch(ast_id(type))
  {
    case TK_NOMINAL:
    {
      const char* type_name = ast_name(ast_childidx(type, 1));
      ast_t* def = ast_get(type, type_name, NULL);

      frame_push(&opt->check, ast_nearest(def, TK_PACKAGE));
      const char* s = package_hygienic_id(&opt->check);
      frame_pop(&opt->check);

      printbuf(buf, "%s_%s", type_name, s);
      return;
    }

    default:
      pony_assert(0);
      return;
  }
}

// Generate a hygienic name for an object of a given type
static const char* object_hygienic_name(pass_opt_t* opt, ast_t* type)
{
  printbuf_t* buf = printbuf_new();
  construct_object_hygienic_name(buf, opt, type);
  const char* r = stringtab(buf->m);
  printbuf_free(buf);
  return r;
}

bool construct_object(pass_opt_t* opt, ast_t* from, ast_t** result)
{
  pony_assert(from != NULL);
  ast_t* type = ast_type(from);
  pony_assert(ast_id(type) == TK_NOMINAL);
  const char* type_name = ast_name(ast_childidx(type, 1));
  const char* object_name = object_hygienic_name(opt, type);

  ast_t* class_def = ast_get(from, type_name, NULL);
  pony_assert(class_def != NULL);

  if(ast_id(class_def) == TK_ACTOR)
  {
    ast_error(opt->check.errors, from,
              "cannot construct compile-time actors");
    return false;
  }

  BUILD_NO_DECL(*result, from,
    NODE(TK_CONSTANT_OBJECT, AST_SCOPE ID(object_name)))
  ast_settype(*result, ast_dup(type));

  ast_t* members = ast_childidx(class_def, 4);
  ast_t* member = ast_child(members);
  while(member != NULL)
  {
    token_id member_id = ast_id(member);
    // Store all the fields that appear in this object, their values can be
    // found in the symbol table.
    if(member_id == TK_FVAR || member_id == TK_FLET || member_id == TK_EMBED)
    {
      sym_status_t s;
      ast_t* member_ast = ast_get(class_def, ast_name(ast_child(member)), &s);
      pony_assert(member_ast != NULL);

      pony_assert(ast_set(*result, ast_name(ast_child(member)), member_ast, s,
                  false));
      ast_append(*result, member);
    }

    member = ast_sibling(member);
  }

  return true;
}

static bool evaluate_method(pass_opt_t* opt, ast_t* this, ast_t* expression,
  ast_t** result)
{
  pony_assert(expression != NULL);
  AST_GET_CHILDREN(expression, postfix, positional, named, question);

  // Named arguments have already been converted to positional
  pony_assert(ast_id(named) == TK_NONE);

  // Check if this is a parametric function type and if so keep hold
  // of the typeargs for later.
  ast_t* typeargs = NULL;
  if(ast_id(ast_childidx(postfix, 1)) == TK_TYPEARGS)
  {
    typeargs = ast_childidx(postfix, 1);
    postfix = ast_child(postfix);
  }

  AST_GET_CHILDREN(postfix, receiver, method_id);

  ast_t* evaluated_receiver;
  if(!evaluate(opt, this, receiver, &evaluated_receiver))
    return false;

  ast_t* receiver_type = ast_type(evaluated_receiver);
  // Look throw the this type arrow
  if((ast_id(receiver_type) == TK_ARROW) &&
    (ast_id(ast_child(receiver_type)) == TK_THISTYPE))
    receiver_type = ast_childidx(receiver_type, 1);

  // We cannot evaluate compile-time behaviours, we shouldn't get here as we
  // cannot construct compile-time actors.
  if(ast_id(postfix) == TK_BEREF)
  {
    ast_error(opt->check.errors, postfix,
              "cannot evaluate compile-time behaviours");
    return false;
  }

  // If there was no built-in then lookup the rdefintion of the function:
  // we push the package from where the function came from so that the lookup
  // proceeds as if we were in the correct package as we may, through private
  // methods within public methods, require access to private memebers.
  ast_t* def
    = ast_get(postfix, ast_name(ast_childidx(receiver_type, 1)), NULL);
  frame_push(&opt->check, ast_nearest(def, TK_PACKAGE));
  deferred_reification_t* method_def = lookup(opt, def, receiver_type,
                                              ast_name(method_id));
  pony_assert(method_def != NULL);
  ast_t* method = deferred_reify(method_def, method_def->ast, opt);
  pony_assert(method != NULL);
  deferred_reify_free(method_def);
  frame_pop(&opt->check);

  if(typeargs != NULL)
  {
    ast_t* typeparams = ast_childidx(method, 2);
    ast_t* r_method = reify(method, typeparams, typeargs, opt, true);
    ast_free_unattached(method);
    method = r_method;
    pony_assert(method != NULL);
  }

  // Evaluate all the arguments and assign them to the repsective paramter
  // names, we also build up an array of the arguments in case we're going to
  // call a builtin.
  ast_t* parameters = ast_childidx(method, 3);
  ast_t* parameter = ast_child(parameters);
  ast_t* argument = ast_child(positional);
  ast_t* evaluated_args[ast_childcount(parameters)];
  size_t i = 0;
  while(argument != NULL && parameter != NULL)
  {
    ast_t* evaluated_arg;
    if(!evaluate(opt, this, argument, &evaluated_arg))
      return false;

    evaluated_args[i++] = evaluated_arg;
    assign_value(opt, method, parameter, evaluated_arg, NULL);
    parameter = ast_sibling(parameter);
    argument = ast_sibling(argument);
  }
  pony_assert(argument == NULL && parameter == NULL);

  // First lookup to see if we have a builtin method to evaluate the expression
  method_ptr_t builtin_method
    = methodtab_lookup(evaluated_receiver, receiver_type, ast_name(method_id));
  if(builtin_method != NULL)
    return builtin_method(opt, evaluated_receiver, evaluated_args, result);

  // Now in a position to evaluate the body
  ast_t* method_body = ast_childidx(method, 6);

  // If this is a method execute the body. If the receiver is an integer/bool
  // type then, even if this is a constructor, just execute the body as we don't
  // want to construcut any object in this case (there would be nothing to put
  // in it - so we just use the raw values).
  if(ast_id(method) != TK_NEW ||
     is_integer(receiver_type) || is_bool(receiver_type))
    return evaluate(opt, evaluated_receiver, method_body, result);

  // If the method is a constructor then we need to build a compile time object
  // adding the values of the fields as the children and setting the type
  // to be the return type of the function body
  if(!construct_object(opt, receiver, result))
    return false;

  // Now run the constructor to initialise the fields etc.
  this = *result;
  if(!evaluate(opt, this, method_body, result))
    return false;

  *result = this;
  return true;
}

static bool evaluate(pass_opt_t* opt, ast_t* this, ast_t* expression,
  ast_t** result)
{
  pony_assert(expression != NULL);

  switch (ast_id(expression))
  {
    // Get and evaluate the inner expression
    case TK_CONSTANT:
      return evaluate(opt, this, ast_child(expression), result);

    // Literal cases where we can return the value
    case TK_NONE:
    case TK_TRUE:
    case TK_FALSE:
    case TK_INT:
    case TK_FLOAT:
    case TK_ERROR:
      *result = ast_dup(expression);
      return true;

    case TK_TYPEREF:
      *result = expression;
      return true;

    // FIXME:
    // String literals are a bit special; we want to construct an object with
    // all of the fields.
    case TK_STRING:
    {
      pony_assert(0);
    }

   // FVARREF may seem unintuitive to add but in fact it's safe due to the deep
   // immutability of val. Once we have constructed a compile-time object it
   // becomes val once it leaves the compile-time expression so the var field
   // is also constant.
   case TK_FVARREF:
   case TK_FLETREF:
    {
      AST_GET_CHILDREN(expression, receiver, id);
      ast_t* evaluated_receiver;
      if(!evaluate(opt, this, receiver, &evaluated_receiver))
        return false;

      *result = ast_getvalue(evaluated_receiver, ast_name(id));
      if(*result == NULL)
      {
        ast_error(opt->check.errors, expression,
          "field is not a compile-time expression");
        return false;
      }
      pony_assert(*result != NULL);
      return true;
    }

    // Variable lookups, checking that the variable has been bound to a value.
    // A missing binding means that the variable is not a compile-time variable.
    case TK_VARREF:
    case TK_PARAMREF:
    case TK_LETREF:
    {
      *result = ast_getvalue(expression, ast_name(ast_child(expression)));
      if(*result == NULL)
      {
        ast_error(opt->check.errors, expression,
          "variable is not a compile-time expression");
        return false;
      }
      pony_assert(*result != NULL);
      return true;
    }

    // Evaluating a destructive read
    case TK_ASSIGN:
    {
      AST_GET_CHILDREN(expression, left, right);

      ast_t* evaluated_right;
      if(!evaluate(opt, this, right, &evaluated_right))
        return false;

      assign_value(opt, this, left, evaluated_right, result);
      return true;
    }

    // Iterate through the sequence and evaluate in order, bailing if we error
    // or returning the last result
    case TK_SEQ:
    {
      ast_t* evaluated;
      for(ast_t* p = ast_child(expression); p != NULL; p = ast_sibling(p))
      {
        if(!evaluate(opt, this, p, &evaluated))
          return false;;
      }
      *result = evaluated;
      return true;
    }

    case TK_CALL:
      return evaluate_method(opt, this, expression, result);

    case TK_THIS:
      *result = this != NULL ? this : expression;
      return true;

    case TK_IF:
    case TK_ELSEIF:
    {
      AST_GET_CHILDREN(expression, condition, then_branch, else_branch);
      ast_t* evaluated_condition;
      if(!evaluate(opt, this, condition, &evaluated_condition))
        return false;

      pony_assert(ast_id(evaluated_condition) == TK_TRUE ||
                  ast_id(evaluated_condition) == TK_FALSE);

      return ast_id(evaluated_condition) == TK_TRUE ?
            evaluate(opt, this, then_branch, result):
            evaluate(opt, this, else_branch, result);
    }

    case TK_WHILE:
    {
      AST_GET_CHILDREN(expression, condition, then_body, else_body);
      ast_t* evaluated_condition;
      if(!evaluate(opt, this, condition, &evaluated_condition))
        return false;

      pony_assert(ast_id(evaluated_condition) == TK_TRUE ||
                  ast_id(evaluated_condition) == TK_FALSE);

      // The condition didn't hold on the first iteration so we evaluate the
      // else
      if(ast_id(evaluated_condition) == TK_FALSE)
        return evaluate(opt, this, else_body, result);

      // The condition held so evaluate the while returning the iteration
      // result as the evaluated result
      while(ast_id(evaluated_condition) == TK_TRUE)
      {
        if(!evaluate(opt, this, then_body, result) ||
           !evaluate(opt, this, condition, &evaluated_condition))
          return false;

        pony_assert(ast_id(evaluated_condition) == TK_TRUE ||
                    ast_id(evaluated_condition) == TK_FALSE);
      }

      return true;
    }

    case TK_TRY:
    {
      // Evaluate the try expression but this may result in a TK_ERROR result,
      // so test if this is the case after evaluation, if so evaluate the else
      // branch
      AST_GET_CHILDREN(expression, try_body, else_body);
      if(!evaluate(opt, this, try_body, result))
        return false;

      if(ast_id(*result) == TK_ERROR)
        return evaluate(opt, this, else_body, result);

      return true;
    }

    case TK_CONSUME:
      return evaluate(opt, this, ast_childidx(expression, 1), result);

    case TK_RECOVER:
      return evaluate(opt, this, ast_childidx(expression, 1), result);

    default:
      ast_error(opt->check.errors, expression,
        "expression was not a compile-time expression");
#ifdef DEBUG
      ast_error_continue(opt->check.errors, expression,
        "unsupported expression token was %s", lexer_print(ast_id(expression)));
#endif
      return false;
  }

  pony_assert(0);
  return false;
}

ast_result_t pass_evaluate(ast_t** astp, pass_opt_t* options)
{
  ast_t* ast = *astp;

  if(ast_id(ast) == TK_CONSTANT)
  {
    ast_t* result;
    if(!evaluate(options, NULL, ast, &result))
    {
      pony_assert(errors_get_count(options->check.errors) > 0);
      return AST_ERROR;
    }

    if(ast_id(result) == TK_ERROR)
    {
      ast_error(options->check.errors, ast,
                "unresolved error occurred during evaluation");
      ast_error_continue(options->check.errors, result,
                         "error originated here");
      return AST_ERROR;
    }

    pony_assert(result != NULL);

    if(ast_id(ast_parent(ast)) == TK_ASSIGN)
      bind_value(options, ast_previous(ast), result, NULL);
    ast_replace(astp, result);
  }

  return AST_OK;
}

ast_result_t pre_pass_evaluate(ast_t** astp, pass_opt_t* options)
{
  (void) options;
  ast_t* ast = *astp;

  if(ast_id(ast) == TK_FLET)
  {
    AST_GET_CHILDREN(ast, f_id, f_type, f_init);
    if(ast_id(f_init) != TK_CONSTANT)
      return AST_OK;

    ast_t* result;
    if(!evaluate(options, NULL, f_init, &result))
    {
      pony_assert(errors_get_count(options->check.errors) > 0);
      return AST_ERROR;
    }

    pony_assert(bind_value(options, ast, result, NULL));
    ast_swap(f_init, ast_from(f_init, TK_NONE));
  }

  return AST_OK;
}
