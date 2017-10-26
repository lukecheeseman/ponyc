#include "evaluate.h"
#include "../type/assemble.h"
#include "ponyassert.h"

bool expr_constant(pass_opt_t* opt, ast_t* ast)
{
  (void)opt;
  pony_assert(ast_id(ast) == TK_CONSTANT);

  ast_t* expression = ast_child(ast);
  ast_settype(ast, control_type_add_branch(opt, NULL, expression));

  return true;
}

ast_result_t pass_evaluate(ast_t** astp, pass_opt_t* options)
{
  (void)astp;
  bool r = true;

  if(!r)
  {
    pony_assert(errors_get_count(options->check.errors) > 0);
    return AST_ERROR;
  }

  return AST_OK;
}
