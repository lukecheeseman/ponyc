#include "builtin_int.h"
#include "../../ast/ast.h"
#include "../../ast/astbuild.h"
#include "../../type/assemble.h"
#include "ponyassert.h"

bool int_create(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  (void) receiver;
  (void) opt;
  *result = ast_child(args);
  return true;
}

typedef void (*binary_int_operation_t)(lexint_t*, lexint_t*, lexint_t*);

static bool int_check_operands(ast_t* lhs_arg, ast_t* rhs_arg, pass_opt_t* opt)
{
  if(lhs_arg == NULL || rhs_arg == NULL)
    return false;

  if(ast_id(lhs_arg) != TK_INT)
  {
    ast_error(opt->check.errors, rhs_arg,
      "%s is not a compile-time integer expression", ast_get_print(lhs_arg));
    return false;
  }

  if(ast_id(rhs_arg) != TK_INT)
  {
    ast_error(opt->check.errors, rhs_arg,
      "%s is not a compile-time integer expression", ast_get_print(rhs_arg));
    return false;
  }

  return true; 
}

static bool int_binary_operation(pass_opt_t* opt, ast_t* receiver,
  ast_t* args, ast_t** result, binary_int_operation_t operation)
{
  pony_assert(ast_id(args) == TK_POSITIONALARGS);
  ast_t* lhs_arg = receiver;
  ast_t* rhs_arg = ast_child(args);
  if(!int_check_operands(lhs_arg, rhs_arg, opt))
    return false;

  *result = ast_dup(lhs_arg);
  lexint_t* lhs = ast_int(*result);
  lexint_t* rhs = ast_int(rhs_arg);

  operation(lhs, lhs, rhs);
  return true;
}

bool int_add(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_add);
}

bool int_sub(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_sub);
}

bool int_mul(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  ast_t* rhs_arg = ast_child(args);
  lexint_t* rhs =  ast_int(rhs_arg);
  if(rhs->high != 0)
  {
    // FIXME: can only print 64 bit integers with ast_get_rpint
    // token method for printing should probably be changed then
    ast_error(opt->check.errors, rhs_arg,
      "Value %s is too large for multiplication", ast_get_print(rhs_arg));
    return false;
  }

  return int_binary_operation(opt, receiver, args, result, &lexint_mul);
}

bool int_div(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  ast_t* rhs_arg = ast_child(args);
  lexint_t* rhs =  ast_int(rhs_arg);
  if(rhs->high != 0)
  {
    // FIXME: can only print 64 bit integers with ast_get_rpint
    // token method for printing should probably be changed then
    ast_error(opt->check.errors, rhs_arg,
      "Value %s is too large for division", ast_get_print(rhs_arg));
    return false;
  }

  return int_binary_operation(opt, receiver, args, result, &lexint_div);
}

bool int_neg(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  pony_assert(ast_id(args) == TK_NONE);
  (void) opt;
  if(ast_id(receiver) != TK_INT)
    return false;

  *result = ast_dup(receiver);
  lexint_t* result_int = ast_int(*result);

  lexint_negate(result_int, result_int);
  return true;
}

bool int_and(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_and);
}

bool int_or(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_or);
}

bool int_xor(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_binary_operation(opt, receiver, args, result, &lexint_xor);
}

bool int_min_value(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  (void) opt;
  (void) args;
  // for all the unsigned values the minimum is zero
  *result = ast_from_int(receiver, 0);
  lexint_t* value = ast_int(*result);

  ast_t* type = ast_type(receiver);
  ast_settype(*result, type);

  const char* name = ast_name(ast_childidx(type, 1));
  if(name == stringtab("I8"))
  {
    value->low = 0x80;
    lexint_negate(value, value);
  }
  else if(name == stringtab("I16"))
  {
    value->low = 0x8000;
    lexint_negate(value, value);
  }
  else if(name == stringtab("I32"))
  {
    value->low = 0x80000000;
    lexint_negate(value, value);
  }
  else if(name == stringtab("I64") ||
          name == stringtab("ILong") ||
          name == stringtab("ISize"))
  {
    value->low = 0x8000000000000000;
    lexint_negate(value, value);
  }
  else if(name == stringtab("I128"))
  {
    value->high = 0x8000000000000000;
    lexint_negate(value, value);
  }
  return true;
}

bool int_max_value(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  (void) opt;
  (void) args;
  *result = ast_from_int(receiver, 0);
  lexint_t* value = ast_int(*result);

  ast_t* type = ast_type(receiver);
  ast_settype(*result, type);

  const char* name = ast_name(ast_childidx(type, 1));
  if(name == stringtab("U8"))
    value->low = 0xFF;
  else if(name == stringtab("U16"))
    value->low = 0xFFFF;
  else if(name == stringtab("U32"))
    value->low = 0xFFFFFFFF;
  else if(name == stringtab("I64") ||
          name == stringtab("ULong") ||
          name == stringtab("USize"))
    value->low = 0xFFFFFFFFFFFFFFFF;
  else if(name == stringtab("U128"))
  {
    value->low = 0XFFFFFFFFFFFFFFFF;
    value->high = 0XFFFFFFFFFFFFFFFF;
  }
  else if(name == stringtab("I8"))
    value->low = 0x7F;
  else if(name == stringtab("I16"))
    value->low = 0x7FFF;
  else if(name == stringtab("I32"))
    value->low = 0x7FFFFFFF;
  else if(name == stringtab("I64") ||
          name == stringtab("ILong") ||
          name == stringtab("ISize"))
    value->low = 0x7FFFFFFFFFFFFFFF;
  else if(name == stringtab("I128"))
  {
    value->low = 0XFFFFFFFFFFFFFFFF;
    value->high = 0X7FFFFFFFFFFFFFFF;
  }
  return true;
}

typedef bool (*test_equality_t)(lexint_t*, lexint_t*);

static bool int_inequality(pass_opt_t* opt,
  ast_t* receiver, ast_t* args, ast_t** result, test_equality_t test)
{
  pony_assert(ast_id(args) == TK_POSITIONALARGS);
  ast_t* lhs_arg = receiver;
  ast_t* rhs_arg = ast_child(args);
  if(!int_check_operands(lhs_arg, rhs_arg, opt))
    return false;

  lexint_t* lhs = ast_int(lhs_arg);
  lexint_t* rhs = ast_int(rhs_arg);

  BUILD(compare, lhs_arg, NODE(test(lhs, rhs) ? TK_TRUE : TK_FALSE));
  ast_settype(compare, type_builtin(opt, ast_type(lhs_arg), "Bool"));

  *result = compare; 
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

bool int_eq(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_eq);
}

bool int_ne(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_ne);
}

bool int_lt(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_lt);
}

bool int_le(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_le);
}

bool int_gt(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_gt);
}

bool int_ge(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return int_inequality(opt, receiver, args, result, &test_ge);
}
