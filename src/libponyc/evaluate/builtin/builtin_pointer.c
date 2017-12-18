#include "builtin_pointer.h"
#include "method_table.h"
#include "../evaluate.h"
#include "../../type/subtype.h"
#include "../../../libponyrt/mem/pool.h"
#include "ponyassert.h"

typedef struct pointer {
  uint64_t size;
  ast_t** elements;
} pointer_t;

uint64_t ptr_get_size(pointer_t* ptr)
{
  return ptr->size;
}

ast_t** ptr_get_elements(pointer_t* ptr)
{
  return ptr->elements;
}

bool pointer_create(pass_opt_t* opt, ast_t* receiver, ast_t** args,
                    ast_t** result)
{
  (void) args;
  if(!construct_object(opt, receiver, result))
    return false;

  pointer_t* structure = POOL_ALLOC(pointer_t);
  structure->elements = NULL;
  structure->size = 0;
  ast_setdata(*result, structure);
  return true;
}

bool pointer_alloc(pass_opt_t* opt, ast_t* receiver, ast_t** args,
                   ast_t** result)
{
  if(!construct_object(opt, receiver, result))
    return false;

  lexint_t* size = ast_int(args[0]);
  ast_t** elements = ponyint_pool_alloc_size(sizeof(ast_t*) * (size_t)size->low);
  for(uint64_t i = 0; i < size->low; ++i)
    elements[i] = NULL;

  pointer_t* structure = POOL_ALLOC(pointer_t);
  structure->elements = elements;
  structure->size = size->low;

  ast_setdata(*result, structure);
  return true;
}

bool pointer_realloc(pass_opt_t* opt, ast_t* receiver, ast_t** args,
                     ast_t** result)
{
  (void) opt;
  pony_assert(is_pointer(ast_type(receiver)));
  pointer_t* structure = ast_data(receiver);

  lexint_t* size = ast_int(args[0]);
  ast_t** elements = ponyint_pool_realloc_size(
    sizeof(ast_t*) * (size_t)structure->size,
    sizeof(ast_t*) * (size_t)size->low,
    structure->elements);
  structure->elements = elements;
  structure->size = size->low;
  *result = receiver;
  return true;
}

bool pointer_apply(pass_opt_t* opt, ast_t* receiver, ast_t** args,
                   ast_t** result)
{
  (void) opt;
  pony_assert(is_pointer(ast_type(receiver)));
  pointer_t* structure = ast_data(receiver);
  uint64_t index = ast_int(args[0])->low;
  pony_assert(index < structure->size);
  *result = structure->elements[index];
  return true;
}

bool pointer_update(pass_opt_t* opt, ast_t* receiver, ast_t** args,
                   ast_t** result)
{
  (void) opt;
  pony_assert(is_pointer(ast_type(receiver)));
  pointer_t* structure = ast_data(receiver);
  uint64_t index = ast_int(args[0])->low;
  pony_assert(index < structure->size);
  *result = structure->elements[index];
  structure->elements[index] = args[1];
  return true;
}

bool pointer_offset(pass_opt_t* opt, ast_t* receiver, ast_t** args,
                    ast_t** result)
{
  (void) opt;
  pony_assert(is_pointer(ast_type(receiver)));

  (void) args;
  if(!construct_object(opt, receiver, result))
    return false;

  pointer_t* structure = ast_data(receiver);
  uint64_t index = ast_int(args[0])->low;

  pointer_t* new_structure = POOL_ALLOC(pointer_t);
  new_structure->elements = &structure->elements[index];
  new_structure->size = structure->size - index;
  ast_setdata(*result, new_structure);
  return true;
}

bool pointer_unsafe(pass_opt_t* opt, ast_t* receiver, ast_t** args,
                   ast_t** result)
{
  (void) opt;
  (void) args;
  *result = receiver;
  return true;
}

void builtin_pointer_add_methods()
{
  methodtab_add(stringtab("Pointer"), stringtab("create"), &pointer_create);
  methodtab_add(stringtab("Pointer"), stringtab("_alloc"), &pointer_alloc);
  methodtab_add(stringtab("Pointer"), stringtab("_realloc"), &pointer_realloc);
  methodtab_add(stringtab("Pointer"), stringtab("_apply"), &pointer_apply);
  methodtab_add(stringtab("Pointer"), stringtab("_update"), &pointer_update);
  methodtab_add(stringtab("Pointer"), stringtab("_unsafe"), &pointer_unsafe);
  methodtab_add(stringtab("Pointer"), stringtab("_offset"), &pointer_offset);
}

