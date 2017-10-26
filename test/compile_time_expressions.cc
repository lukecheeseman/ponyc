#include <gtest/gtest.h>
#include <platform.h>
#include <type/subtype.h>
#include "util.h"

#define TEST_COMPILE(src) DO(test_compile(src, "expr"))
#define TEST_ERROR(src) DO(test_error(src, "expr"))

class CompileTimeExpressionTest: public PassTest
{};

// TODO: in a simlar way to other tests, design the functionality
// such that we can unit tests the compile-time expression evaluation

TEST_F(CompileTimeExpressionTest, CompileTimeLiteral)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: Bool = # true";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeExpression)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # (true or false)";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeLiteralTypeInference)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # 3";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeVariable)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # (let y: U32 = 3; y)";

  TEST_COMPILE(src);
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
