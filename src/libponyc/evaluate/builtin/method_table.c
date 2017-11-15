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

static void methodtab_add(const char* name, const char* type, method_ptr_t method)
{
  method_entry_t m = {name, type, method};
  method_table_put(&method_table, method_dup(&m));
}

// TODO: let builtins register their own methods
// Builds the method lookup table for the supported builting operations
void methodtab_init()
{
  // TODO: rename to real?
  // integer operations
  method_table_init(&method_table, 20);
  methodtab_add(stringtab("integer"), stringtab("create"), &int_create);

  methodtab_add(stringtab("integer"), stringtab("add"), &int_add);
  methodtab_add(stringtab("integer"), stringtab("sub"), &int_sub);
  methodtab_add(stringtab("integer"), stringtab("mul"), &int_mul);
  methodtab_add(stringtab("integer"), stringtab("div"), &int_div);

  methodtab_add(stringtab("integer"), stringtab("neg"), &int_neg);
  methodtab_add(stringtab("integer"), stringtab("eq"), &int_eq);
  methodtab_add(stringtab("integer"), stringtab("ne"), &int_ne);
  methodtab_add(stringtab("integer"), stringtab("lt"), &int_lt);
  methodtab_add(stringtab("integer"), stringtab("le"), &int_le);
  methodtab_add(stringtab("integer"), stringtab("gt"), &int_gt);
  methodtab_add(stringtab("integer"), stringtab("ge"), &int_ge);

  methodtab_add(stringtab("integer"), stringtab("op_and"), &int_and);
  methodtab_add(stringtab("integer"), stringtab("op_or"), &int_or);
  methodtab_add(stringtab("integer"), stringtab("op_xor"), &int_xor);
  methodtab_add(stringtab("integer"), stringtab("op_not"), &int_not);
  methodtab_add(stringtab("integer"), stringtab("shl"), &int_shl);
  methodtab_add(stringtab("integer"), stringtab("shr"), &int_shr);

  // integer casting methods
  methodtab_add(stringtab("integer"), stringtab("i8"), &int_i8);
  methodtab_add(stringtab("integer"), stringtab("i16"), &int_i16);
  methodtab_add(stringtab("integer"), stringtab("i32"), &int_i32);
  methodtab_add(stringtab("integer"), stringtab("i64"), &int_i64);
  methodtab_add(stringtab("integer"), stringtab("i128"), &int_i128);
  methodtab_add(stringtab("integer"), stringtab("ilong"), &int_ilong);
  methodtab_add(stringtab("integer"), stringtab("isize"), &int_isize);
  methodtab_add(stringtab("integer"), stringtab("u8"), &int_u8);
  methodtab_add(stringtab("integer"), stringtab("u16"), &int_u16);
  methodtab_add(stringtab("integer"), stringtab("u32"), &int_u32);
  methodtab_add(stringtab("integer"), stringtab("u64"), &int_u64);
  methodtab_add(stringtab("integer"), stringtab("u128"), &int_u128);
  methodtab_add(stringtab("integer"), stringtab("ulong"), &int_ulong);
  methodtab_add(stringtab("integer"), stringtab("usize"), &int_usize);

  methodtab_add(stringtab("integer"), stringtab("clz"), &int_clz);

  //TODO: make the type names consistent in case
  // boolean operations
  methodtab_add(stringtab("Bool"), stringtab("eq"), &bool_eq);
  methodtab_add(stringtab("Bool"), stringtab("ne"), &bool_ne);
  methodtab_add(stringtab("Bool"), stringtab("op_and"), &bool_and);
  methodtab_add(stringtab("Bool"), stringtab("op_or"), &bool_or);
  methodtab_add(stringtab("Bool"), stringtab("op_xor"), &bool_xor);
  methodtab_add(stringtab("Bool"), stringtab("op_not"), &bool_not);

  methodtab_add(stringtab("Pointer"), stringtab("create"), &pointer_create);
  methodtab_add(stringtab("Pointer"), stringtab("_alloc"), &pointer_alloc);
  methodtab_add(stringtab("Pointer"), stringtab("_realloc"), &pointer_realloc);
  methodtab_add(stringtab("Pointer"), stringtab("_apply"), &pointer_apply);
  methodtab_add(stringtab("Pointer"), stringtab("_update"), &pointer_update);
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
