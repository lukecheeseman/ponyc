#ifndef BUILTIN_BOOL_H
#define BUILTIN_BOOL_H

#include "../../ast/ast.h"
#include "../../pass/pass.h"

bool bool_create(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

bool bool_eq(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool bool_ne(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

bool bool_and(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool bool_or(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool bool_xor(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool bool_not(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

#endif
