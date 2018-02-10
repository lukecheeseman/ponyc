#ifndef METHOD_TABLE_H
#define METHOD_TABLE_H

#include "../../ast/ast.h"
#include "../../pass/pass.h"

typedef struct builtin_context builtin_context_t;

typedef struct method_entry_t method_entry_t;

typedef bool (*method_ptr_t)(pass_opt_t* opt, ast_t* receiver, ast_t** args,
  ast_t** result);

void builtin_init(pass_opt_t* opt);

void builtin_done(pass_opt_t* opt);

void builtin_add(pass_opt_t* opt, const char* name, const char* type,
  method_ptr_t method);

method_ptr_t builtin_lookup(pass_opt_t* opt, ast_t* receiver, ast_t* type,
  const char* operation);

long long unsigned int builtin_sizeof(pass_opt_t* opt, ast_t* type);

#endif
