#include "builtin_int.h"
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
