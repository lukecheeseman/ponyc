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
