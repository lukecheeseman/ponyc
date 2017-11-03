#include "builtin_bool.h"
#include "../../ast/ast.h"
#include "../../ast/astbuild.h"
#include "../../type/assemble.h"
#include "ponyassert.h"

static bool bool_check_operand(ast_t* arg)
{
  bool is_bool = (ast_id(arg) == TK_TRUE) || (ast_id(arg) == TK_FALSE);
  pony_assert(is_bool);
  return is_bool;
}

bool bool_create(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  (void) receiver;
  (void) opt;
  *result = ast_child(args);
  return bool_check_operand(*result);
}

typedef bool (*test_equality_t)(token_id, token_id);

static bool bool_logical_op(pass_opt_t* opt,
  ast_t* receiver, ast_t* args, ast_t** result, test_equality_t test)
{
  pony_assert(ast_id(args) == TK_POSITIONALARGS);
  ast_t* lhs_arg = receiver;
  ast_t* rhs_arg = ast_child(args);
  if(!bool_check_operand(lhs_arg) || !bool_check_operand(rhs_arg))
    return false;

  token_id lhs = ast_id(lhs_arg);
  token_id rhs = ast_id(rhs_arg);

  BUILD_NO_DECL(*result, lhs_arg, NODE(test(lhs, rhs) ? TK_TRUE : TK_FALSE));
  ast_settype(*result, type_builtin(opt, ast_type(lhs_arg), "Bool"));
  return true;
}

static bool test_eq(token_id lhs, token_id rhs)
{
  return lhs == rhs;
}

static bool test_ne(token_id lhs, token_id rhs)
{
  return lhs != rhs;
}

static bool test_and(token_id lhs, token_id rhs)
{
  return lhs == TK_TRUE && rhs == TK_TRUE;
}

static bool test_or(token_id lhs, token_id rhs)
{
  return lhs == TK_TRUE || rhs == TK_TRUE;
}

bool bool_eq(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return bool_logical_op(opt, receiver, args, result, &test_eq);
}

bool bool_ne(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return bool_logical_op(opt, receiver, args, result, &test_ne);
}

bool bool_and(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return bool_logical_op(opt, receiver, args, result, &test_and);
}

bool bool_or(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return bool_logical_op(opt, receiver, args, result, &test_or);
}

bool bool_xor(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  return bool_logical_op(opt, receiver, args, result, &test_ne);
}

bool bool_not(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result)
{
  (void) opt;
  (void) args;
  if(!bool_check_operand(receiver))
    return false; 

  BUILD_NO_DECL(*result, receiver, NODE(ast_id(receiver) == TK_FALSE ? TK_TRUE : TK_FALSE));
  ast_settype(*result, type_builtin(opt, ast_type(receiver), "Bool"));
  return true;
}
