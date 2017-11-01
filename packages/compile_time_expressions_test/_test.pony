"""
# Compile Time Expression Tests
"""
use "ponytest"
use "collections"

actor Main is TestList
  new create(env: Env) => PonyTest(env, this)
  new make() => None

  fun tag tests(test: PonyTest) =>
    test(_TestLiterals)
    test(_TestSeq)
    test(_TestNegation)
    test(_TestArithmetic)
    test(_TestChainedArithmetic)
    test(_TestCompileTimeAssignment)
    test(_TestCompileTimeVariable)
    test(_TestIntegerEquivalence)
    /*
    test(_TestIntegerToIntegerTypeCast)
    test(_TestBitwise)
    test(_TestBoolLogicalOperations)
    test(_TestFunctionCall)
    test(_TestFunctionCallNamedArgs)
    test(_TestCompileTimeObjectField)
    test(_TestCompileTimeObjectMethod)
    test(_TestCompileTimeObjectEmbeddedField)
    test(_TestCompileTimeWhileLoop)
    test(_TestCompileTimeScoping)
    test(_TestCompileTimeTuples)
    */

class iso _TestLiterals is UnitTest

  fun name(): String => "CompileTimeExpression/Literals"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](# 1, 1)
    h.assert_eq[U32](# 42, 42)
    h.assert_eq[I64](# 1, 1)
    h.assert_eq[I64](# 42, 42)
    h.assert_eq[Bool](# true, true)
    h.assert_eq[Bool](# false, false)


class iso _TestSeq is UnitTest

  fun name(): String => "CompileTimeExpression/Seq"

  fun apply(h: TestHelper) =>
    h.assert_eq[Bool](# (true; false; false), (true; false; false))
    h.assert_eq[Bool](# (true; true), (true; true))

class iso _TestNegation is UnitTest

  fun name(): String => "CompileTimeExpression/Negation"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(-1), -1)
    h.assert_eq[I32](#(-1), -1)

class iso _TestArithmetic is UnitTest
  """
  Test compile time U64 arithmertic
  """
  fun name(): String => "CompileTimeExpression/U64Arithmetic"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(42 + 8), 42 + 8)
    h.assert_eq[U32](#(42 - 8), 42 - 8)
    h.assert_eq[U32](#(42 * 8), 42 * 8)
    h.assert_eq[U32](#(42 / 8), 42 / 8)

class iso _TestChainedArithmetic is UnitTest

  fun name(): String => "CompileTimeExpression/ChainedArithmetic"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(-2 + (1 - 9)), -2 + (1 - 9))
    h.assert_eq[I32](#(-2 + (1 - 9)), -2 + (1 - 9))
    h.assert_eq[I32](#(-2 + 1), -2 + 1)
    h.assert_eq[I32](#(1 + -2), 1 + -2)
    h.assert_eq[I32](#(-1 + 2), -1 + 2)
    h.assert_eq[I32](#(2 + -1), 2 + -1)
    h.assert_eq[I32](#(-1 - -1), -1 - -1)
    h.assert_eq[I32](#(-1 - 1), -1 - 1)

class iso _TestCompileTimeAssignment is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeAssignment"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(let y: U32 = 4; y), 4)
    h.assert_eq[U32](#(let y: U32 = 4; y * 2), 8)
    h.assert_eq[U32](#(var y: U32 = 4; y = y * 2; y), 8)
    h.assert_eq[U32](#(var y: U32 = 4; y = y * 2; y + 4), 12)

class iso _TestCompileTimeVariable is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeVariable"

  fun apply(h: TestHelper) =>
    let x: U32 = # (1 + 2)
    let y: U32 = # (x * 2)
    h.assert_eq[U32]((1+2)*2, # y)

class iso _TestIntegerEquivalence is UnitTest

  fun name(): String => "CompileTimeExpression/IntegerEquivalence"

  fun apply(h: TestHelper) =>
    h.assert_true(#(U32(1) == U32(1)))
    h.assert_false(#(U32(1) == U32(7)))
    h.assert_true(#(I32(-1) == I32(-1)))
    h.assert_false(#(I32(-1) == I32(-7)))
    h.assert_true(#(I32(1) == -I32(-1)))

    h.assert_false(#(U32(1) != U32(1)))
    h.assert_true(#(U32(1) != U32(7)))
    h.assert_false(#(I32(-1) != I32(-1)))
    h.assert_true(#(I32(-1) != I32(-7)))
    h.assert_false(#(I32(1) != -I32(-1)))

    h.assert_false(#(U32(1) > U32(1)))
    h.assert_false(#(U32(1) > U32(7)))
    h.assert_true(#(U32(7) > U32(1)))
    h.assert_false(#(I32(-1) > I32(-1)))
    h.assert_true(#(I32(-1) > I32(-7)))
    h.assert_false(#(I32(-7) > I32(-1)))
    h.assert_false(#(I32(1) > -I32(-1)))

    h.assert_true(#(U32(1) >= U32(1)))
    h.assert_false(#(U32(1) >= U32(7)))
    h.assert_true(#(U32(7) >= U32(1)))
    h.assert_true(#(I32(-1) >= I32(-1)))
    h.assert_true(#(I32(-1) >= I32(-7)))
    h.assert_false(#(I32(-7) >= I32(-1)))
    h.assert_true(#(I32(1) >= -I32(-1)))

    h.assert_false(#(U32(1) < U32(1)))
    h.assert_true(#(U32(1) < U32(7)))
    h.assert_false(#(U32(7) < U32(1)))
    h.assert_false(#(I32(-1) < I32(-1)))
    h.assert_false(#(I32(-1) < I32(-7)))
    h.assert_true(#(I32(-7) < I32(-1)))
    h.assert_false(#(I32(1) < -I32(-1)))

    h.assert_true(#(U32(1) <= U32(1)))
    h.assert_true(#(U32(1) <= U32(7)))
    h.assert_false(#(U32(7) <= U32(1)))
    h.assert_true(#(I32(-1) <= I32(-1)))
    h.assert_false(#(I32(-1) <= I32(-7)))
    h.assert_true(#(I32(-7) <= I32(-1)))
    h.assert_true(#(I32(1) <= -I32(-1)))

/*
class iso _TestIntegerToIntegerTypeCast is UnitTest
  """
  Test casting one integer type to another as
  compile time expression
  """

  fun name(): String => "CompileTimeExpression/IntegerCast"

  fun apply(h: TestHelper) =>
    h.assert_eq[I8](#(U32(4).i8()), 4)
    h.assert_eq[I16](#(U32(4).i16()), 4)
    h.assert_eq[I32](#(U32(4).i32()), 4)
    h.assert_eq[I64](#(U32(4).i64()), 4)
    h.assert_eq[I128](#(U32(4).i128()), 4)
    h.assert_eq[ILong](#(U32(4).ilong()), 4)
    h.assert_eq[ISize](#(U32(4).isize()), 4)
    h.assert_eq[U8](#(U32(4).u8()), 4)
    h.assert_eq[U16](#(U32(4).u16()), 4)
    h.assert_eq[U32](#(U32(4).u32()), 4)
    h.assert_eq[U64](#(U32(4).u64()), 4)
    h.assert_eq[U128](#(U32(4).u128()), 4)
    h.assert_eq[ULong](#(U32(4).ulong()), 4)
    h.assert_eq[USize](#(U32(4).usize()), 4)

    h.assert_eq[I8](#(I128(73).i8()), 73)
    h.assert_eq[I16](#(I128(73).i16()), 73)
    h.assert_eq[I32](#(I128(73).i32()), 73)
    h.assert_eq[I64](#(I128(73).i64()), 73)
    h.assert_eq[I128](#(I128(73).i128()), 73)
    h.assert_eq[ILong](#(I128(73).ilong()), 73)
    h.assert_eq[ISize](#(I128(73).isize()), 73)
    h.assert_eq[U8](#(I128(73).u8()), 73)
    h.assert_eq[U16](#(I128(73).u16()), 73)
    h.assert_eq[U32](#(I128(73).u32()), 73)
    h.assert_eq[U64](#(I128(73).u64()), 73)
    h.assert_eq[U128](#(I128(73).u128()), 73)
    h.assert_eq[ULong](#(I128(73).ulong()), 73)
    h.assert_eq[USize](#(I128(73).usize()), 73)


class iso _TestBitwise is UnitTest

  fun name(): String => "CompileTimeExpression/U64Bitwise"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(3823890482 and 123931), 3823890482 and 123931)
    h.assert_eq[U32](#(3823890482 or 123931), 3823890482 or 123931)
class iso _TestBoolLogicalOperations is UnitTest

  fun name(): String => "CompileTimeExpression/Bool"

  fun apply(h: TestHelper) =>
    h.assert_false((# false or false))
    h.assert_true((# false or true))
    h.assert_true((# true or false))
    h.assert_true((# true or true))

    h.assert_false((# false and false))
    h.assert_false((# false and true))
    h.assert_false((# true and false))
    h.assert_true((# true and true))

class iso _TestFunctionCall is UnitTest

  fun name(): String => "CompileTimeExpression/fib"

  fun fib(n: U32): U32 =>
    if n == 0 then
      0
    elseif n <= 2 then
      1
    else
      fib(n - 2) + fib(n - 1)
    end

   fun apply(h: TestHelper) =>
      h.assert_eq[U32](#fib(1), fib(1))
      h.assert_eq[U32](#fib(8), fib(8))
      h.assert_eq[U32](#fib(20), fib(20))

class iso _TestFunctionCallNamedArgs is UnitTest

  fun name(): String => "CompileTimeExpression/named"

  fun foo(x: U32, y: U32, z:U32 = 4): U32 => (x * y) + z

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#foo(1, 2, 3), foo(1, 2, 3))
    h.assert_eq[U32](#foo(1 where y=2, z=3), foo(1 where y=2, z=3))
    h.assert_eq[U32](#foo(1 where z=2, y=3), foo(1 where z=2, y=3))
    h.assert_eq[U32](#foo(1 where y=3), foo(1 where y=3))

class ClassWithField
  let f: U32

  new val create(f': U32) => f = f'

class iso _TestCompileTimeObjectField is UnitTest

  fun name(): String => "CompileTimeEfpression/CompileTimeObjectField"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(ClassWithField(48).f), ClassWithField(48).f)
    h.assert_eq[U32](#(ClassWithField(48)).f, ClassWithField(48).f)

class ClassWithFieldAndApply
  let f: U32

  new val create(f': U32) => f = f'

  fun apply(): U32 => f + f + f

class iso _TestCompileTimeObjectMethod is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeObjectMethod"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(ClassWithFieldAndApply(48)()), ClassWithFieldAndApply(48)())
    h.assert_eq[U32](#(ClassWithFieldAndApply(48))(), ClassWithFieldAndApply(48)())

class ClassWithEmbeddedField
  embed ef: ClassWithField val

  new val create(f: U32) => ef = ClassWithField(f)

class iso _TestCompileTimeObjectEmbeddedField is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeObjectiEmbeddedField"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(ClassWithEmbeddedField(12).ef.f), ClassWithEmbeddedField(12).ef.f)

    let static_c = #(ClassWithEmbeddedField(982))
    let dynamic_c = ClassWithEmbeddedField(982)
    h.assert_eq[U32](static_c.ef.f, dynamic_c.ef.f)

    let static_ef = #(ClassWithEmbeddedField(78).ef)
    let dynamic_ef = ClassWithEmbeddedField(78).ef
    h.assert_eq[U32](static_ef.f, dynamic_ef.f)

    let static_f = #(ClassWithEmbeddedField(2123).ef.f)
    let dynamic_f = ClassWithEmbeddedField(2123).ef.f
    h.assert_eq[U32](static_f, dynamic_f)

class iso _TestCompileTimeWhileLoop is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeWhileLoop"

  fun apply(h: TestHelper) =>
    let x = #(
      var result: U32 = 1
      var i: U32 = 1
      while i < 10 do
        result = result * (i = i + 1)
      end
      result)
    let y = (
      var result: U32 = 1
      var i: U32 = 1
      while i < 10 do
        result = result * (i = i + 1)
      end
      result)
    h.assert_eq[U32](x, y)

class iso _TestCompileTimeScoping is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeScoping"

  fun apply(h: TestHelper) =>
    let x = #(
      var z: U32 = 4
      if true then z = 5 end
      z)
    let y = (
      var z: U32 = 4
      if true then z = 5 end
      z)
    h.assert_eq[U32](x, y)

class iso _TestCompileTimeTuples is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeTuples"

  fun apply(h: TestHelper) =>
    let x: (U32, String) = # (12, "Hello")
    h.assert_eq[U32](# x._1, 12)
    h.assert_eq[String](# x._2, "Hello")
    h.assert_eq[U32](# x._1, x._1)
    h.assert_eq[String](# x._2, x._2)
*/
