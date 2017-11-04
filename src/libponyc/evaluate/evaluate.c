#include "evaluate.h"
#include "builtin/method_table.h"
#include "../ast/lexer.h"
#include "../type/assemble.h"
#include "../type/lookup.h"
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
static bool assign_value(pass_opt_t* opt, ast_t* left, ast_t* right,
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
      return true;

    default:
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
  if(left_id == TK_VAR || left_id == TK_VARREF)
    return false;

  return assign_value(opt, left, right, result);
}


static bool evaluate(pass_opt_t* opt, ast_t* expression, ast_t** result);

static bool evaluate_method(pass_opt_t* opt, ast_t* expression,
  ast_t* arguments, ast_t** result)
{
  pony_assert(expression != NULL);
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

  // TODO: Need to handle parametric functions at some point (and objects for that matter)
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
  ast_t* def
    = ast_get(expression, ast_name(ast_childidx(receiver_type, 1)), NULL);
  frame_push(&opt->check, ast_nearest(def, TK_PACKAGE));
  ast_t* method = lookup(opt, def, receiver_type, ast_name(method_id));
  pony_assert(method != NULL);
  frame_pop(&opt->check);

  ast_t* parameters = ast_childidx(method, 3);

  ast_t* parameter = ast_child(parameters);
  ast_t* argument = ast_child(arguments);
  while(argument != NULL)
  {
    assign_value(opt, parameter, argument, NULL);
    argument = ast_sibling(argument);
    parameter = ast_sibling(parameter);
  }

  // Evaluate the body
  ast_t* method_body = ast_childidx(method, 6);
  if(!evaluate(opt, method_body, result))
    return false;

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
      return evaluate(opt, right, &evaluated_right) &&
             assign_value(opt, left, evaluated_right, result);
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

    pony_assert(result != NULL);
    ast_replace(astp, result);

    ast = *astp;
    if(ast_id(ast_parent(ast)) == TK_ASSIGN)
      bind_value(options, ast_previous(ast), ast, NULL);
  }

  return AST_OK;
}
