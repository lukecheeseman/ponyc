#include <gtest/gtest.h>
#include <platform.h>
#include <type/subtype.h>
#include "util.h"

#define TEST_COMPILE(src) DO(test_compile(src, "codegen"))
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

TEST_F(CompileTimeExpressionTest, CompileTimeFunctionMissingArgs)
{
  const char* src =
    "actor Main\n"
    "  fun foo(n: U32): U32 => n\n"
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
  const char* src =
    "class C1\n"
    "  let x: U32 = 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c: C1 val = # C1";

  TEST_EVALUATE_COMPILE(src);
  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeCompileTimeObjectWithVarField)
{
  const char* src =
    "class C1\n"
    "  var x: U32 = 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c = # C1";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeCompileTimeObjectVarFieldReassign)
{
  const char* src =
    "class C1\n"
    "  var x: U32 = 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c = # (let o = C1\n"
    "     o.x = 2\n"
    "     consume o)";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, ClassWithBoundLetField)
{
  const char* src =
    "class C1\n"
    "  let x: U32 = # 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) => true";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, ClassWithBoundLetFieldUsedInMethod)
{
  const char* src =
    "class C1\n"
    "  let x: U32 = # 2\n"
    "\n"
    "  fun apply(): U32 => # (x + 10)\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) => true";

  set_builtin(NULL);
  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, ClassWithBoundLetFieldUsedInField)
{
  const char* src =
    "class C1\n"
    "  let x: U32 = # 10\n"
    "  let y: U32 = # (x + 2)\n"
    "\n"
    "  fun apply(): U32 => # (y + 10)\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) => true";

  set_builtin(NULL);
  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, ClassWithUnitiliasedLetFieldUsedInField)
{
  const char* src =
    "class C1\n"
    "  let x: U32\n"
    "\n"
    "  new create() => x = # 2\n"
    "\n"
    "  fun foo(): U32 => # (x + 10)\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) => true";

  set_builtin(NULL);
  TEST_ERROR(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeCompileTimeResultIsVal)
{
  const char* src =
    "class C1\n"
    "  let x: U32 = 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c: C1 ref = # C1";

  TEST_ERROR(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeCompileTimeObjectNoAlias)
{
  // Test that the reach type is added for C1
  const char* src =
    "class C1\n"
    "  let x: U32 = 2\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    # C1";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeCompileTimeTuple)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let t: (U32, U32, U32) = # (1, 2, 3)";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, CompileTimeNonBuiltinIntConstructor)
{
  const char* src =
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x = # (U32(2).max_value() - 1)";

  set_builtin(NULL);
  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, NestedObjectsWithRepeatedFieldName)
{
  const char* src =
    "class C1\n"
    "  let f: U32\n"
    "  new create(f': U32) => f = f'\n"
    "\n"
    "class C2\n"
    "  let f: C1\n"
    "  new create(x: U32) => f = C1(x)\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c: C2 val = # C2(4)";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, ParametricConstructor)
{
  const char* src =
    "class C1[A]\n"
    "  new create[B](x: B) => true\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c = # C1[U32].create[Bool](true)";

  TEST_COMPILE(src);
}

TEST_F(CompileTimeExpressionTest, FunctionChaining)
{
  const char* src =
    "class C1\n"
    "  var x: U32 = 1\n"
    "\n"
    "  fun ref apply(x': U32) => x = consume x'\n"
    "\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let x: U32 = # (C1.>apply(2).>apply(72).x)\n";

  TEST_COMPILE(src);
}
