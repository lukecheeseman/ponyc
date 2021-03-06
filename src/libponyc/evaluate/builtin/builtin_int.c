#include "builtin_int.h"
#include "builtin.h"
#include "../../ast/ast.h"
#include "../../ast/astbuild.h"
#include "../../type/assemble.h"
#include "../../type/subtype.h"
#include "ponyassert.h"

#include <memory.h>

static bool int_check_operand(ast_t* ast)
{
  bool is_int = (ast_id(ast) == TK_INT);
  pony_assert(is_int);
  return is_int;
}

bool int_create(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) receiver;
  (void) opt;
  *result = ast_dup(args[0]);
  return int_check_operand(*result);
}

typedef void (*binary_int_operation_t)(lexint_t*, lexint_t*, lexint_t*);

static bool int_binary_operation(pass_opt_t* opt, ast_t* receiver,
  ast_t** args, ast_t** result, binary_int_operation_t operation)
{
  (void)opt;
  ast_t* lhs_arg = receiver;
  ast_t* rhs_arg = args[0];
  if(!int_check_operand(lhs_arg) || !int_check_operand(rhs_arg))
    return false;

  *result = ast_dup(lhs_arg);
  lexint_t* lhs = ast_int(*result);
  lexint_t* rhs = ast_int(rhs_arg);

  operation(lhs, lhs, rhs);
  return true;
}

bool int_add(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_add);
}

bool int_sub(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_sub);
}

bool int_mul(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  ast_t* rhs_arg = args[0];
  lexint_t* rhs =  ast_int(rhs_arg);
  if(rhs->high != 0)
  {
    ast_error(opt->check.errors, rhs_arg,
      "Value %s is too large for multiplication", lexint_string(rhs));
    return false;
  }

  return int_binary_operation(opt, receiver, args, result, &lexint_mul);
}

bool int_div(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  ast_t* rhs_arg = args[0];
  lexint_t* rhs =  ast_int(rhs_arg);
  if(rhs->high != 0)
  {
    ast_error(opt->check.errors, rhs_arg,
      "Value %s is too large for division", lexint_string(rhs));
    return false;
  }

  return int_binary_operation(opt, receiver, args, result, &lexint_div);
}

bool int_neg(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) opt;
  (void) args;
  if(!int_check_operand(receiver))
    return false;

  *result = ast_dup(receiver);
  lexint_t* result_int = ast_int(*result);

  lexint_negate(result_int, result_int);
  return true;
}

bool int_and(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_and);
}

bool int_or(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_or);
}

bool int_xor(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_xor);
}

typedef bool (*test_equality_t)(lexint_t*, lexint_t*);

static bool int_inequality(pass_opt_t* opt,
  ast_t* receiver, ast_t** args, ast_t** result, test_equality_t test)
{
  ast_t* lhs_arg = receiver;
  ast_t* rhs_arg = args[0];
  if(!int_check_operand(lhs_arg) || !int_check_operand(rhs_arg))
    return false;

  lexint_t* lhs = ast_int(lhs_arg);
  lexint_t* rhs = ast_int(rhs_arg);

  BUILD_NO_DECL(*result, lhs_arg, NODE(test(lhs, rhs) ? TK_TRUE : TK_FALSE));
  ast_settype(*result, type_builtin(opt, ast_type(lhs_arg), "Bool"));
  return true;
}

static bool test_eq(lexint_t* lhs, lexint_t* rhs)
{
  return lexint_cmp(lhs, rhs) == 0;
}

static bool test_ne(lexint_t* lhs, lexint_t* rhs)
{
  return lexint_cmp(lhs, rhs) != 0;
}

static bool test_lt(lexint_t* lhs, lexint_t* rhs)
{
  return lexint_cmp(lhs, rhs) < 0;
}

static bool test_le(lexint_t* lhs, lexint_t* rhs)
{
  return lexint_cmp(lhs, rhs) <= 0;
}

static bool test_gt(lexint_t* lhs, lexint_t* rhs)
{
  return lexint_cmp(lhs, rhs) > 0;
}

static bool test_ge(lexint_t* lhs, lexint_t* rhs)
{
  return lexint_cmp(lhs, rhs) >= 0;
}

bool int_eq(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_eq);
}

bool int_ne(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_ne);
}

bool int_lt(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_lt);
}

bool int_le(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_le);
}

bool int_gt(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_gt);
}

bool int_ge(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_ge);
}

bool int_not(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) opt;
  (void) args;
  if(!int_check_operand(receiver))
    return false;

  *result = ast_dup(receiver);
  lexint_t* result_int = ast_int(*result);
  lexint_not(result_int, result_int);
  return true;
}

bool int_shl(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void)opt;
  ast_t* lhs_arg = receiver;
  ast_t* rhs_arg = args[0];
  if(!int_check_operand(lhs_arg) || !int_check_operand(rhs_arg))
    return false;

  *result = ast_dup(lhs_arg);
  lexint_t* lhs = ast_int(*result);
  lexint_t* rhs = ast_int(rhs_arg);

  lexint_shl(lhs, lhs, rhs->low);
  return true;
}

bool int_shr(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void)opt;
  ast_t* lhs_arg = receiver;
  ast_t* rhs_arg = args[0];
  if(!int_check_operand(lhs_arg) || !int_check_operand(rhs_arg))
    return false;

  *result = ast_dup(lhs_arg);
  lexint_t* lhs = ast_int(*result);
  lexint_t* rhs = ast_int(rhs_arg);

  lexint_shr(lhs, lhs, rhs->low);
  return true;
}

// casting methods
static bool int_cast_to_type(pass_opt_t* opt, ast_t* receiver, ast_t** result,
  const char* type)
{
  if(!int_check_operand(receiver))
    return false;

  *result = ast_dup(receiver);
  ast_t* new_type = type_builtin(opt, ast_type(receiver), type);
  pony_assert(new_type != NULL);
  ast_settype(receiver, new_type);
  return true;
}

bool int_i8(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "I8");
}

bool int_i16(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "I16");
}

bool int_i32(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "I32");
}

bool int_i64(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "I64");
}

bool int_i128(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "I128");
}

bool int_ilong(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "ILong");
}

bool int_isize(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "ISize");
}

bool int_u8(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "U8");
}

bool int_u16(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "U16");
}

bool int_u32(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "U32");
}

bool int_u64(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "U64");
}

bool int_u128(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "U128");
}

bool int_ulong(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "ULong");
}

bool int_usize(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) args;
  return int_cast_to_type(opt, receiver, result, "USize");
}

bool int_clz(pass_opt_t* opt, ast_t* receiver, ast_t** args, ast_t** result)
{
  (void) opt;
  (void) args;
  if(!int_check_operand(receiver))
    return false;

  uint64_t bits = 8 * builtin_sizeof(opt, ast_type(receiver));

  lexint_t mask;
  lexint_zero(&mask);
  mask.low = 1;
  lexint_shl(&mask, &mask, (bits - 1));
  lexint_sub64(&mask, &mask, 1);

  lexint_t val;
  memcpy(&val, ast_int(receiver), sizeof(lexint_t));
  uint64_t n = 0;
  while(lexint_cmp64(&val, 0) == 1)
  {
    lexint_shl(&val, &val, 1);
    lexint_and(&val, &val, &mask);
    n++;
  }

  *result = ast_from_int(receiver, n);
  ast_settype(*result, ast_type(receiver));
  return true;
}

void builtin_int_add_methods(pass_opt_t* opt)
{
  builtin_add(opt, "integer", "create", &int_create);

  builtin_add(opt, "integer", "add", &int_add);
  builtin_add(opt, "integer", "sub", &int_sub);
  builtin_add(opt, "integer", "mul", &int_mul);
  builtin_add(opt, "integer", "div", &int_div);

  builtin_add(opt, "integer", "neg", &int_neg);
  builtin_add(opt, "integer", "eq", &int_eq);
  builtin_add(opt, "integer", "ne", &int_ne);
  builtin_add(opt, "integer", "lt", &int_lt);
  builtin_add(opt, "integer", "le", &int_le);
  builtin_add(opt, "integer", "gt", &int_gt);
  builtin_add(opt, "integer", "ge", &int_ge);

  builtin_add(opt, "integer", "op_and", &int_and);
  builtin_add(opt, "integer", "op_or", &int_or);
  builtin_add(opt, "integer", "op_xor", &int_xor);
  builtin_add(opt, "integer", "op_not", &int_not);
  builtin_add(opt, "integer", "shl", &int_shl);
  builtin_add(opt, "integer", "shr", &int_shr);

  builtin_add(opt, "integer", "i8", &int_i8);
  builtin_add(opt, "integer", "i16", &int_i16);
  builtin_add(opt, "integer", "i32", &int_i32);
  builtin_add(opt, "integer", "i64", &int_i64);
  builtin_add(opt, "integer", "i128", &int_i128);
  builtin_add(opt, "integer", "ilong", &int_ilong);
  builtin_add(opt, "integer", "isize", &int_isize);
  builtin_add(opt, "integer", "u8", &int_u8);
  builtin_add(opt, "integer", "u16", &int_u16);
  builtin_add(opt, "integer", "u32", &int_u32);
  builtin_add(opt, "integer", "u64", &int_u64);
  builtin_add(opt, "integer", "u128", &int_u128);
  builtin_add(opt, "integer", "ulong", &int_ulong);
  builtin_add(opt, "integer", "usize", &int_usize);

  builtin_add(opt, "integer", "clz", &int_clz);
}

