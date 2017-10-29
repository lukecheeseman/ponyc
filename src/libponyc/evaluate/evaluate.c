#include "evaluate.h"
#include "../ast/lexer.h"
#include "../type/assemble.h"
#include "ponyassert.h"

bool expr_constant(pass_opt_t* opt, ast_t* ast)
{
  pony_assert(ast_id(ast) == TK_CONSTANT);

  ast_t* expression = ast_child(ast);
  ast_settype(ast, control_type_add_branch(opt, NULL, expression));

  return true;
}

static ast_t* evaluate(pass_opt_t* opt, ast_t* expression)
{
  pony_assert(expression != NULL);

  switch (ast_id(expression))
  {
    // Get and evaluate the inner expression
    case TK_CONSTANT:
      return evaluate(opt, ast_child(expression));

    // Literal cases where we can return the value
    case TK_NONE:
    case TK_TRUE:
    case TK_FALSE:
    case TK_INT:
    case TK_FLOAT:
    case TK_ERROR:
    case TK_STRING:
      return expression;

    case TK_SEQ:
    {
      ast_t* evaluated = NULL;
      for(ast_t* p = ast_child(expression); p != NULL; p = ast_sibling(p))
        evaluated = evaluate(opt, p);
      return evaluated;
    }

    default:
      ast_error(opt->check.errors, expression,
        "expression interpeter does not support this expression");
#ifdef DEBUG
      ast_error_continue(opt->check.errors, expression,
        "unsupported expression token was %s", lexer_print(ast_id(expression)));
#endif
      return expression;
  }

  pony_assert(0);
  return expression;
}

ast_result_t pass_evaluate(ast_t** astp, pass_opt_t* options)
{
  ast_t* ast = *astp;
  if (ast_id(ast) == TK_CONSTANT)
  {
    ast_t* evaluated = evaluate(options, ast);
    ast_replace(astp, evaluated);
  }

  // TODO: At some point, we will need to consider mapping assignments

  if(!true)
  {
    pony_assert(errors_get_count(options->check.errors) > 0);
    return AST_ERROR;
  }

  return AST_OK;
}
