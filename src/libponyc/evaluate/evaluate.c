#include "evaluate.h"
#include "builtin/method_table.h"
#include "../ast/astbuild.h"
#include "../ast/lexer.h"
#include "../ast/printbuf.h"
#include "../type/assemble.h"
#include "../type/lookup.h"
#include "../type/reify.h"
#include "../pkg/package.h"
#include "ponyassert.h"

void evaluate_init(pass_opt_t* opt)
{
  if(opt->limit >= PASS_EVALUATE)
    methodtab_init();
}

bool expr_constant(pass_opt_t* opt, ast_t* ast)
{
  pony_assert(ast_id(ast) == TK_CONSTANT);

  ast_t* expression = ast_child(ast);
  ast_settype(ast, control_type_add_branch(opt, NULL, expression));

  return true;
}

/*
 * Construct a mapping from the given lhs identifier to the rhs value. This
 * method assumes that all mappings are valid, the method is provided as a
 * convenience to extract the identifier name.
 */
static void assign_value(pass_opt_t* opt, ast_t* left, ast_t* right,
  ast_t** result)
{
  (void) opt;
  pony_assert(left != NULL);
  pony_assert(right != NULL);

  switch(ast_id(left))
  {
    case TK_VAR:
    case TK_VARREF:
    case TK_LET:
    case TK_PARAM:
      pony_assert(ast_setvalue(left, ast_name(ast_child(left)), right, result));
      return;

    case TK_FLETREF:
    case TK_FVARREF:
    {
      const char* name = ast_name(ast_childidx(left, 1));
      pony_assert(ast_setvalue(left, name, right, result));
      return;
    }

    default:
      pony_assert(0);
  }
}

// Bind a value so that it can be used in later compile-time expressions
// FIXME: We should also be able bind a let field
static bool bind_value(pass_opt_t* opt, ast_t* left, ast_t* right,
  ast_t** result)
{
  (void) opt;
  pony_assert(left != NULL);
  pony_assert(right != NULL);

  token_id left_id = ast_id(left);
  if(left_id == TK_VAR || left_id == TK_VARREF)
    return false;

  assign_value(opt, left, right, result);
  return true;
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

static bool evaluate(pass_opt_t* opt, ast_t* expression, ast_t** result);

static bool evaluate_method(pass_opt_t* opt, ast_t* expression,
  ast_t* arguments, ast_t** result)
{
  pony_assert(expression != NULL);

  // Check if this is a parametric function type and if so keep hold
  // of the typeargs for later.
  ast_t* typeargs = NULL;
  if(ast_id(ast_childidx(expression, 1)) == TK_TYPEARGS)
  {
    typeargs = ast_childidx(expression, 1);
    expression = ast_child(expression);
  }

  AST_GET_CHILDREN(expression, receiver, method_id);

  ast_t* evaluated_receiver;
  if(!evaluate(opt, receiver, &evaluated_receiver))
    return false;

  ast_t* receiver_type = ast_type(evaluated_receiver);
  // Look throw the this type arrow
  if((ast_id(receiver_type) == TK_ARROW) &&
    (ast_id(ast_child(receiver_type)) == TK_THISTYPE))
    receiver_type = ast_childidx(receiver_type, 1);

  // First lookup to see if we have a builtin method to evaluate the expression
  method_ptr_t builtin_method
    = methodtab_lookup(evaluated_receiver, receiver_type, ast_name(method_id));
  if(builtin_method != NULL)
    return builtin_method(opt, evaluated_receiver, arguments, result);

  // We cannot evaluate compile-time behaviours, we shouldn't get here as we
  // cannot construct compile-time actors.
  if(ast_id(expression) == TK_BEREF)
  {
    ast_error(opt->check.errors, expression,
              "cannot evaluate compile-time behaviours");
    return false;
  }

  // If there was no built-in then lookup the rdefintion of the function:
  // we push the package from where the function came from so that the lookup
  // proceeds as if we were in the correct package as we may, through private
  // methods within public methods, require access to private memebers.
  ast_t* def = ast_get(expression, ast_name(ast_childidx(receiver_type, 1)), NULL);
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

  // Evaluate all the arguments and assign them to the repsecitive paramter
  // names
  ast_t* parameters = ast_childidx(method, 3);
  ast_t* parameter = ast_child(parameters);
  ast_t* argument = ast_child(arguments);
  while(argument != NULL)
  {
    assign_value(opt, parameter, argument, NULL);
    argument = ast_sibling(argument);
    parameter = ast_sibling(parameter);
  }

  // Now in a position to evaluate the body
  ast_t* method_body = ast_childidx(method, 6);
  if(!evaluate(opt, method_body, result))
    return false;

  // If the method is a constructor then we need to build a compile time object
  // adding the values of the fields as the children and setting the type
  // to be the return type of the function body
  if(ast_id(method) == TK_NEW)
  {
    const char* type_name = ast_name(ast_childidx(receiver_type, 1));
    const char* object_name = object_hygienic_name(opt, receiver_type);

    ast_t* class_def = ast_get(expression, type_name, NULL);
    pony_assert(class_def != NULL);

    if(ast_id(class_def) == TK_ACTOR)
    {
      ast_error(opt->check.errors, method,
                "cannot construct compile-time actors");
      return false;
    }

    // Get the return type
    ast_t* return_type = ast_childidx(ast_type(expression), 3);
    BUILD_NO_DECL(*result, expression,
      NODE(TK_CONSTANT_OBJECT, ID(object_name)))
    ast_set_symtab(*result, symtab_dup(ast_get_symtab(method)));
    ast_settype(*result, ast_dup(return_type));

    ast_t* members = ast_childidx(class_def, 4);
    ast_t* member = ast_child(members);
    while(member != NULL)
    {
      if(ast_id(member) == TK_FVAR)
      {
        ast_error(opt->check.errors, member,
                  "compile time objects fields must be read-only");
        return false;
      }
      member = ast_sibling(member);
    }
  }

  return true;
}

static bool evaluate(pass_opt_t* opt, ast_t* expression, ast_t** result)
{
  pony_assert(expression != NULL);

  switch (ast_id(expression))
  {
    // Get and evaluate the inner expression
    case TK_CONSTANT:
      return evaluate(opt, ast_child(expression), result);

    // Literal cases where we can return the value
    case TK_NONE:
    case TK_TRUE:
    case TK_FALSE:
    case TK_INT:
    case TK_FLOAT:
    case TK_ERROR:
    case TK_STRING:
      *result = expression;
      return true;

    case TK_TYPEREF:
      *result = expression;
      return true;

    // Evaluating a destructive read
    case TK_ASSIGN:
    {
      AST_GET_CHILDREN(expression, left, right);

      ast_t* evaluated_right;
      if(!evaluate(opt, right, &evaluated_right))
        return false;

      assign_value(opt, left, evaluated_right, result);
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

      return true;
    }

    // Iterate through the sequence and evaluate in order, bailing if we error
    // or returning the last result
    case TK_SEQ:
    {
      ast_t* evaluated;
      for(ast_t* p = ast_child(expression); p != NULL; p = ast_sibling(p))
      {
        if(!evaluate(opt, p, &evaluated))
          return false;;
      }
      *result = evaluated;
      return true;
    }

    case TK_CALL:
    {
      AST_GET_CHILDREN(expression, postfix, positional, named, question);

      // Named arguments have already been converted to positional
      pony_assert(ast_id(named) == TK_NONE);

      // Build up the evaluated arguments
      ast_t* evaluated_args = ast_from(positional, ast_id(positional));
      ast_t* argument = ast_child(positional);
      while(argument != NULL)
      {
        ast_t* evaluated_arg;
        if(!evaluate(opt, argument, &evaluated_arg))
          return false;

        ast_append(evaluated_args, evaluated_arg);
        argument = ast_sibling(argument);
      }

      return evaluate_method(opt, postfix, evaluated_args, result);
    }

    case TK_THIS:
      *result = expression;
      return true;

    case TK_IF:
    case TK_ELSEIF:
    {
      AST_GET_CHILDREN(expression, condition, then_branch, else_branch);
      ast_t* evaluated_condition;
      if(!evaluate(opt, condition, &evaluated_condition))
        return false;

      pony_assert(ast_id(evaluated_condition) == TK_TRUE ||
                  ast_id(evaluated_condition) == TK_FALSE);

      return ast_id(evaluated_condition) == TK_TRUE ?
            evaluate(opt, then_branch, result):
            evaluate(opt, else_branch, result);
    }

    case TK_WHILE:
    {
      AST_GET_CHILDREN(expression, condition, then_body, else_body);
      ast_t* evaluated_condition;
      if(!evaluate(opt, condition, &evaluated_condition))
        return false;

      pony_assert(ast_id(evaluated_condition) == TK_TRUE ||
                  ast_id(evaluated_condition) == TK_FALSE);

      // The condition didn't hold on the first iteration so we evaluate the
      // else
      if(ast_id(evaluated_condition) == TK_FALSE)
        return evaluate(opt, else_body, result);

      // The condition held so evaluate the while returning the iteration
      // result as the evaluated result
      while(ast_id(evaluated_condition) == TK_TRUE)
      {
        if(!evaluate(opt, then_body, result) ||
           !evaluate(opt, condition, &evaluated_condition))
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
      if(!evaluate(opt, try_body, result))
        return false;

      if(ast_id(*result) == TK_ERROR)
        return evaluate(opt, else_body, result);

      return true;
    }

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
    if(!evaluate(options, ast, &result))
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
    ast_replace(astp, result);

    ast = *astp;
    if(ast_id(ast_parent(ast)) == TK_ASSIGN)
      bind_value(options, ast_previous(ast), ast, NULL);
  }

  return AST_OK;
}
