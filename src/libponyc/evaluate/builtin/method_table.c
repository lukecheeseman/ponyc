#include "method_table.h"
#include "builtin_bool.h"
#include "builtin_int.h"
#include "builtin_pointer.h"
#include "../../type/subtype.h"
#include "../../../libponyrt/mem/pool.h"
#include "../../../libponyrt/ds/hash.h"

#include <string.h>

static method_entry_t* method_dup(method_entry_t* method)
{
  method_entry_t* m = POOL_ALLOC(method_entry_t);
  memcpy(m, method, sizeof(method_entry_t));
  return m;
}

static size_t method_hash(method_entry_t* method)
{
  return ponyint_hash_ptr(method->name) ^ ponyint_hash_ptr(method->name);
}

static bool method_cmp(method_entry_t* a, method_entry_t* b)
{
  return a->name == b->name && a->type == b->type;
}

static void method_free(method_entry_t* method)
{
  POOL_FREE(method_entry_t, method);
}

DEFINE_HASHMAP(method_table, method_table_t, method_entry_t, method_hash,
  method_cmp, method_free);

static method_table_t method_table;

void methodtab_add(const char* name, const char* type, method_ptr_t method)
{
  method_entry_t m = {name, type, method};
  method_table_put(&method_table, method_dup(&m));
}

// TODO: let builtins register their own methods
// Builds the method lookup table for the supported builting operations
void methodtab_init()
{
  method_table_init(&method_table, 20);
  builtin_int_add_methods();
  builtin_bool_add_methods();
  builtin_pointer_add_methods();
}

method_ptr_t methodtab_lookup(ast_t* receiver, ast_t* type,
  const char* operation)
{
  const char* type_name =
    is_integer(type) || ast_id(receiver) == TK_INT ? "integer" :
    is_float(type) || ast_id(receiver) == TK_FLOAT ? "float" :
    ast_name(ast_childidx(type, 1));

  method_entry_t m1 = {stringtab(type_name), stringtab(operation), NULL};
  size_t index = HASHMAP_UNKNOWN;
  method_entry_t* m2 = method_table_get(&method_table, &m1, &index);

  if(m2 == NULL)
    return NULL;

  return m2->method;
}
