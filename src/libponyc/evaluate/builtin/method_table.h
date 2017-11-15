#ifndef METHOD_TABLE_H
#define METHOD_TABLE_H

#include "../../ast/ast.h"
#include "../../pass/pass.h"

typedef bool (*method_ptr_t)(pass_opt_t* opt, ast_t*, ast_t**, ast_t**);

typedef struct method_entry_t
{
  const char* type;
  const char* name;
  const method_ptr_t method;
} method_entry_t;

DECLARE_HASHMAP(method_table, method_table_t, method_entry_t);

void methodtab_init();

method_ptr_t methodtab_lookup(ast_t* receiver, ast_t* type,
  const char* operation);

#endif
