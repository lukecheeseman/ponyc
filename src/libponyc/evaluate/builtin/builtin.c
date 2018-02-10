#include "builtin.h"
#include "builtin_bool.h"
#include "builtin_int.h"
#include "builtin_pointer.h"
#include "ponyassert.h"
#include "../evaluate.h"
#include "../../codegen/codegen.h"
#include "../../codegen/gentype.h"
#include "../../type/subtype.h"
#include "../../../libponyrt/mem/pool.h"
#include "../../../libponyrt/ds/hash.h"

#include <string.h>

typedef struct method_entry_t
{
  const char* type;
  const char* name;
  const method_ptr_t method;
} method_entry_t;

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

DECLARE_HASHMAP(method_table, method_table_t, method_entry_t);

DEFINE_HASHMAP(method_table, method_table_t, method_entry_t, method_hash,
  method_cmp, method_free);

typedef struct builtin_context {
  method_table_t method_table; 
  compile_t c;
} builtin_context_t;

void builtin_add(pass_opt_t* opt, const char* name, const char* type,
  method_ptr_t method)
{
  method_entry_t m = {stringtab(name), stringtab(type), method};
  method_table_put(&opt->builtin_ctx->method_table, method_dup(&m));
}

// Initialises the data for builtin operations; this includes constructing the 
// lookup table for builtin functions as well as initialising some target
// specific information.
void builtin_init(pass_opt_t* opt)
{
  opt->builtin_ctx = POOL_ALLOC(builtin_context_t);

  method_table_init(&opt->builtin_ctx->method_table, 20);
  builtin_int_add_methods(opt);
  builtin_bool_add_methods(opt);
  builtin_pointer_add_methods(opt);

  compile_t* c = &opt->builtin_ctx->c;
  codegen_init_target_information(c, opt, "compile-time", false);
  codegen_init_datatypes(c);
}

void builtin_done(pass_opt_t* opt)
{
  method_table_destroy(&opt->builtin_ctx->method_table);
  POOL_FREE(builtin_context_t, opt->builtin_ctx);
}

method_ptr_t builtin_lookup(pass_opt_t* opt, ast_t* receiver, ast_t* type,
  const char* operation)
{
  const char* type_name =
    is_integer(type) || ast_id(receiver) == TK_INT ? "integer" :
    is_float(type) || ast_id(receiver) == TK_FLOAT ? "float" :
    ast_name(ast_childidx(type, 1));

  method_entry_t m1 = {stringtab(type_name), stringtab(operation), NULL};
  size_t index = HASHMAP_UNKNOWN;
  method_entry_t* m2 = method_table_get(&opt->builtin_ctx->method_table, &m1,
                                        &index);

  if(m2 == NULL)
    return NULL;

  return m2->method;
}

long long unsigned int builtin_sizeof(pass_opt_t* opt, ast_t* type)
{
  pony_assert(ast_id(type) == TK_NOMINAL);
  const char* type_name = ast_name(ast_childidx(type, 1));
  LLVMTypeRef target_type = codegen_get_primitive_type(&opt->builtin_ctx->c, type_name);
  return LLVMABISizeOfType(opt->builtin_ctx->c.target_data, target_type);
}

