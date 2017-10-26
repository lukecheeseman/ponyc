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
