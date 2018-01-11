#ifndef EVALUATE_H
#define EVALUATE_H

#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

void evaluate_init(pass_opt_t* opt);

void evaluate_done(pass_opt_t* opt);

bool expr_constant(pass_opt_t* opt, ast_t* ast);

bool construct_object(pass_opt_t* opt, ast_t* from, ast_t** result);

ast_result_t pass_evaluate(ast_t** astp, pass_opt_t* options);

ast_result_t pre_pass_evaluate(ast_t** astp, pass_opt_t* options);

PONY_EXTERN_C_END

#endif
