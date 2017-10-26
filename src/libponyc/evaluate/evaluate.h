#ifndef EVALUATE_H
#define EVALUATE_H

#include "../ast/ast.h"
#include "../pass/pass.h"

bool expr_constant(pass_opt_t* opt, ast_t* ast);

ast_result_t pass_evaluate(ast_t** astp, pass_opt_t* options);

#endif
