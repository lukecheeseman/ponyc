#ifndef EVALUATE_H
#define EVALUATE_H

#include "builtin/builtin.h"
#include "../ast/ast.h"
#include "../codegen/codegen.h"

PONY_EXTERN_C_BEGIN

typedef struct pass_opt_t pass_opt_t;

void evaluate_init(pass_opt_t* opt);

void evaluate_done(pass_opt_t* opt);

bool expr_constant(pass_opt_t* opt, ast_t* ast);

bool construct_object(pass_opt_t* opt, ast_t* from, ast_t** result);

ast_result_t pass_evaluate(ast_t** astp, pass_opt_t* options);

ast_result_t pre_pass_evaluate(ast_t** astp, pass_opt_t* options);

PONY_EXTERN_C_END

#endif
