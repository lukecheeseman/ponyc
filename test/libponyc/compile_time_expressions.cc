#include <gtest/gtest.h>
#include <platform.h>
#include <type/subtype.h>
#include "util.h"

#define TEST_EVALUATE_COMPILE(src) DO(test_compile(src, "evaluate"))
#define TEST_TYPE_COMPILE(src) DO(test_compile(src, "expr"))
#define TEST_ERROR(src) DO(test_error(src, "evaluate"))

class CompileTimeExpressionTest: public PassTest
{};

TEST_F(CompileTimeExpressionTest, CompileTimeLiteral)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: Bool = # true";

  TEST_EVALUATE_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeLiteralTypeInference)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # 3";

  TEST_EVALUATE_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeSequenceOfLiterals)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: Bool = # (true; false; true)";

  TEST_TYPE_COMPILE(src);
}


TEST_F(CompileTimeExpressionTest, CompileTimeExpression)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: Bool = # (true or false)";

  set_builtin(NULL);
  TEST_TYPE_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeVariable)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # (let y: U32 = 3; y)";

  TEST_TYPE_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeVariableScope)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # (let y: U32 = 3; y)\n"
    "    let z: U32 = y";

  TEST_ERROR(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeExpressionAssignedToLetAndUsed)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # 2\n"
    "    let y: U32 = # x";

  TEST_EVALUATE_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeExpressionAssignedToVarAndUsed)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # 2\n"
    "    var y: U32 = # x\n"
    "    let z: U32 = # y";

  TEST_ERROR(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeError)
{
  const char* src =
    "actor Main\n"
    "  fun foo(): U32 ? => if true then error else 32 end\n"
    "\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # foo()";

  TEST_ERROR(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeErrorNotPartial)
{
  // Test that evaluating an expression at compile-time means
  // we can lose the partialness of a function
  const char* src =
    "actor Main\n"
    "  fun foo(): U32 ? => if false then error else 32 end\n"
    "\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # foo()";

  TEST_EVALUATE_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeCompileTimeObjectEvaluates)
{
  // This test is designed to ensure the compile-time object doesn't raise an
  // error during evaluation. Not that a valid object gets created in the
  // backend.
  const char* src =
    "class C1\n"
    "  let x: U32 = 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c = # C1";

  TEST_EVALUATE_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeCompileTimeObjectWithVarField)
{
  // This test is designed to ensure the compile-time object doesn't raise an
  // error during evaluation. Not that a valid object gets created in the
  // backend.
  const char* src =
    "class C1\n"
    "  var x: U32 = 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c = # C1";

  TEST_ERROR(src);
}
