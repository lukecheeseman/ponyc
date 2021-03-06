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
    test(_TestCompileTimeTuples)
    test(_TestSeq)
    test(_TestNegation)
    test(_TestArithmetic)
    test(_TestChainedArithmetic)
    test(_TestCompileTimeAssignment)
    test(_TestCompileTimeVariable)
    test(_TestIntegerEquivalence)
    test(_TestIntegerBitwise)
    test(_TestIntegerToIntegerTypeCast)
    test(_TestBoolLogicalOperations)
    test(_TestConditional)
    test(_TestWhileLoop)
    test(_TestScoping)
    test(_TestTry)
    test(_TestFunctionCall)
    test(_TestFunctionCallWithArgs)
    test(_TestFunctionCallWithNamedArgs)
    test(_TestRecursiveFunction)
    test(_TestParametricFunction)
    test(_TestCompileTimeObjectField)
    test(_TestCompileTimeObjectMethod)
    test(_TestCompileTimeObjectNestedClassField)
    test(_TestCompileTimeObjectEmbeddedField)
    test(_TestFunctionChaining)
    test(_TestCompileTimePrimitive)
    test(_TestCompileTimeString)

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

class iso _TestIntegerBitwise is UnitTest

  fun name(): String => "CompileTimeExpression/IntegerBitwise"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(3823890482 and 123931), 3823890482 and 123931)
    h.assert_eq[U32](#(3823890482 or 123931), 3823890482 or 123931)
    h.assert_eq[U32](#(3823890482 xor 123931), 3823890482 xor 123931)
    h.assert_eq[U32](#(not 3823890482), not 3823890482)

    h.assert_eq[I32](#(42 and 27), 42 and 27)
    h.assert_eq[I32](#(-42 and 27), -42 and 27)
    h.assert_eq[I32](#(42 and -27), 42 and -27)
    h.assert_eq[I32](#(-42 and -27), -42 and -27)

    h.assert_eq[I32](#(42 or 27), 42 or 27)
    h.assert_eq[I32](#(-42 or 27), -42 or 27)
    h.assert_eq[I32](#(42 or -27), 42 or -27)
    h.assert_eq[I32](#(-42 or -27), -42 or -27)

    h.assert_eq[I32](#(42 xor 27), 42 xor 27)
    h.assert_eq[I32](#(-42 xor 27), -42 xor 27)
    h.assert_eq[I32](#(42 xor -27), 42 xor -27)
    h.assert_eq[I32](#(-42 xor -27), -42 xor -27)

    h.assert_eq[I32](#(not 42), not 42)
    h.assert_eq[I32](#(not -42), not -42)

    h.assert_eq[U32](#(10 << 23), 10 << 23)
    h.assert_eq[U32](#(23 << 10), 23 << 10)

    h.assert_eq[U32](#(10 >> 23), 10 >> 23)
    h.assert_eq[U32](#(23 >> 10), 23 >> 10)

    h.assert_eq[I32](#(10 << 23), 10 << 23)
    h.assert_eq[I32](#(-10 << 23), -10 << 23)

    h.assert_eq[I32](#(23 << 10), 23 << 10)
    h.assert_eq[I32](#(-23 << 10), -23 << 10)

    h.assert_eq[I32](#(10 >> 23), 10 >> 23)
    h.assert_eq[I32](#(-10 >> 23), -10 >> 23)

    h.assert_eq[I32](#(23 >> 10), 23 >> 10)
    h.assert_eq[I32](#(-23 >> 10), -23 >> 10)

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

class iso _TestBoolLogicalOperations is UnitTest

  fun name(): String => "CompileTimeExpression/BoolLogicalOperations"

  fun apply(h: TestHelper) =>
    h.assert_false((# false))
    h.assert_true((# true))

    h.assert_true((# (not false)))
    h.assert_false((# (not true)))

    h.assert_false((# (false or false)))
    h.assert_true((# (false or true)))
    h.assert_true((# (true or false)))
    h.assert_true((# (true or true)))

    h.assert_false((# (false and false)))
    h.assert_false((# (false and true)))
    h.assert_false((# (true and false)))
    h.assert_true((# (true and true)))

    h.assert_false((# (false xor false)))
    h.assert_true((# (false xor true)))
    h.assert_true((# (true xor false)))
    h.assert_false((# (true xor true)))

    h.assert_true((# (false == false)))
    h.assert_false((# (false == true)))
    h.assert_false((# (true == false)))
    h.assert_true((# (true == true)))

    h.assert_false((# (false != false)))
    h.assert_true((# (false != true)))
    h.assert_true((# (true != false)))
    h.assert_false((# (true != true)))

class iso _TestConditional is UnitTest

  fun name(): String => "CompileTimeExpression/Conditional"

   fun apply(h: TestHelper) =>
      h.assert_eq[U32](#(if true then 2 else 5 end),
                        (if true then 2 else 5 end))
      h.assert_eq[U32](#(if false then 2 else 5 end),
                        (if false then 2 else 5 end))

      let cond_true: Bool = # true
      h.assert_eq[U32](#(if cond_true then 2 else 5 end),
                        (if cond_true then 2 else 5 end))
      let cond_false: Bool = # false
      h.assert_eq[U32](#(if cond_false then 2 else 5 end),
                        (if cond_false then 2 else 5 end))

      h.assert_eq[U32](#(if true then 2 elseif true then 5 else 62 end),
                        (if true then 2 elseif true then 5 else 62 end))
      h.assert_eq[U32](#(if true then 2 elseif false then 5 else 62 end),
                        (if true then 2 elseif false then 5 else 62 end))
      h.assert_eq[U32](#(if false then 2 elseif true then 5 else 62 end),
                        (if false then 2 elseif true then 5 else 62 end))
      h.assert_eq[U32](#(if false then 2 elseif false then 5 else 62 end),
                        (if false then 2 elseif false then 5 else 62 end))

class iso _TestWhileLoop is UnitTest

  fun name(): String => "CompileTimeExpression/WhileLoop"

  fun test_while(h: TestHelper) =>
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

  fun test_else(h: TestHelper) =>
    let x = #(
      var result: U32 = 1
      while false do
        result = result * 2
      else
        result = 4
      end
      result)
    let y = (
      var result: U32 = 1
      while false do
        result = result * 2
      else
        result = 4
      end
      result)
    h.assert_eq[U32](x, y)
    h.assert_eq[U32](x, 4)

  fun apply(h: TestHelper) =>
    test_while(h)
    test_else(h)


class iso _TestScoping is UnitTest

  fun name(): String => "CompileTimeExpression/Scoping"

  fun apply(h: TestHelper) =>
    var x = #(
      var z: U32 = 4
      if true then z = 5 end
      z)
    var y = (
      var z: U32 = 4
      if true then z = 5 end
      z)
    h.assert_eq[U32](x, y)
    h.assert_eq[U32](x, 5)

class iso _TestTry is UnitTest

  fun name(): String => "CompileTimeExpression/Try"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(try if true then 2 else error end else 3 end),
                      (try if true then 2 else error end else 3 end))
    h.assert_eq[U32](#(try if false then 2 else error end else 3 end),
                      (try if false then 2 else error end else 3 end))

class iso _TestFunctionCall is UnitTest

  fun name(): String => "CompileTimeExpression/FunctionCall"

  fun foo(): U32 => 16

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](# (foo()), foo())

class iso _TestFunctionCallWithArgs is UnitTest

  fun name(): String => "CompileTimeExpression/FunctionCallWithArgs"

  fun foo(b: Bool, n: U32): U32 => if b then n else 2 end

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](# (foo(true, 64)), foo(true, 64))
    h.assert_eq[U32](# (foo(false, 64)), foo(false, 64))

class iso _TestRecursiveFunction is UnitTest

  fun name(): String => "CompileTimeExpression/RecursiveFunction"

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

class iso _TestFunctionCallWithNamedArgs is UnitTest

  fun name(): String => "CompileTimeExpression/FunctionCallWithNamedArgs"

  fun foo(x: U32, y: U32, z:U32 = 4): U32 => (x * y) + z

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#foo(1, 2, 3), foo(1, 2, 3))
    h.assert_eq[U32](#foo(1 where y=2, z=3), foo(1 where y=2, z=3))
    h.assert_eq[U32](#foo(1 where z=2, y=3), foo(1 where z=2, y=3))
    h.assert_eq[U32](#foo(1 where y=3), foo(1 where y=3))

class iso _TestParametricFunction is UnitTest

  fun name(): String => "CompileTimeExpression/ParametricFunction"

  fun foo[A](x: A): A => x

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#foo[U32](53), foo[U32](53))
    h.assert_eq[U32](#foo[U32](9), foo[U32](9))
    h.assert_eq[Bool](#foo[Bool](true), foo[Bool](true))

class ClassWithField is (Equatable[ClassWithField] & Stringable)
  let f: U32

  fun eq(o: ClassWithField box): Bool => f == o.f

  fun string(): String iso^ => f.string()

  new iso create(f': U32) => f = consume f'

class iso _TestCompileTimeObjectField is UnitTest

  fun name(): String => "CompileTimeExression/CompileTimeObjectField"

  fun apply(h: TestHelper) =>
    h.assert_eq[ClassWithField val]((#ClassWithField(48)),
                                    ClassWithField(48))
    h.assert_eq[U32]((#ClassWithField(48)).f, ClassWithField(48).f)

class ClassWithVarField is (Equatable[ClassWithVarField] & Stringable)
  var f: U32

  fun eq(o: ClassWithVarField box): Bool => f == o.f

  fun string(): String iso^ => f.string()

  new iso create(f': U32) => f = consume f'

class iso _TestCompileTimeObjectVarField is UnitTest

  fun name(): String => "CompileTimeExression/CompileTimeObjectVarField"

  fun apply(h: TestHelper) =>
    h.assert_eq[ClassWithVarField val]((#ClassWithVarField(48)),
                                       ClassWithVarField(48))
    h.assert_eq[ClassWithVarField val](
      (# (let c = ClassWithVarField(48)
          c.f = 13
          consume c)),
      (let c = ClassWithVarField(48)
       c.f = 13
       consume c)
      )

    h.assert_eq[U32]((#ClassWithVarField(48)).f,
                     ClassWithVarField(48).f)
    h.assert_eq[U32](
      (# (let c' = ClassWithVarField(48)
          c'.f = 13
          consume c')).f,
      (let c' = ClassWithVarField(48)
       c'.f = 13
       consume c').f
      )

class ClassWithFieldAndApply
  let f: U32

  new val create(f': U32) => f = f'

  fun apply(): U32 => f + f + f

class iso _TestCompileTimeObjectMethod is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeObjectMethod"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(ClassWithFieldAndApply(48)()),
                     ClassWithFieldAndApply(48)())
    h.assert_eq[U32](#(ClassWithFieldAndApply(48))(),
                     ClassWithFieldAndApply(48)())

class ClassWithVarFieldAndApply
  var f: U32

  new create(f': U32) => f = consume f'

  fun ref apply() => f = 601

class iso _TestCompileTimeObjectMethodWritesVar is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeObjectMethodWritesVar"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(let c = ClassWithVarFieldAndApply(48)
                       c()
                       c.f),
                     (let c = ClassWithVarFieldAndApply(48)
                      c()
                      c.f))

class iso _TestCompileTimeTuples is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeTuples"

  fun apply(h: TestHelper) =>
    let x: (U32, Bool) = # (12, true)
    h.assert_eq[U32](# x._1, 12)
    h.assert_eq[Bool](# x._2, true)
    h.assert_eq[U32](# x._1, x._1)
    h.assert_eq[Bool](# x._2, x._2)

class ClassWithClassField
  let cf: ClassWithField

  new create(f: U32) => cf = ClassWithField(f)

class iso _TestCompileTimeObjectNestedClassField is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeObjectNestedClassField"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](#(ClassWithClassField(12).cf.f), ClassWithClassField(12).cf.f)

    let static_c = #(ClassWithClassField(982))
    let dynamic_c = ClassWithClassField(982)
    h.assert_eq[U32](static_c.cf.f, dynamic_c.cf.f)

    let static_cf = #(ClassWithClassField(78).cf)
    let dynamic_cf = ClassWithClassField(78).cf
    h.assert_eq[U32](static_cf.f, dynamic_cf.f)

    let static_f = #(ClassWithClassField(2123).cf.f)
    let dynamic_f = ClassWithClassField(2123).cf.f
    h.assert_eq[U32](static_f, dynamic_f)

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

class ClassWithVarAndIncrement
  var f: U32

  new create(f': U32) => f = consume f'

  fun ref apply() => f = f + 1

class iso _TestFunctionChaining is UnitTest

  fun name(): String => "CompileTimeExpression/FunctionChaining"

  fun apply(h: TestHelper) =>
    h.assert_eq[U32](ClassWithVarAndIncrement(2).>apply().f,
                     # (ClassWithVarAndIncrement(2).>apply().f))
    h.assert_eq[U32](ClassWithVarAndIncrement(2).>apply().>apply().f,
                     # (ClassWithVarAndIncrement(2).>apply().>apply().f))
    h.assert_eq[U32](ClassWithVarAndIncrement(2).>apply().>apply().>apply().f,
                     # (ClassWithVarAndIncrement(2).>apply().>apply().>apply().f))

primitive BasicPrimitive is (Equatable[BasicPrimitive] & Stringable)

  fun string(): String iso^ => "BasicPrimitive".clone()

class iso _TestCompileTimePrimitive is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimePrimitive"

  fun apply(h: TestHelper) =>
    let p1 = BasicPrimitive
    let p2 = # BasicPrimitive
    h.assert_eq[BasicPrimitive](p1, p2)

    let p3 = # BasicPrimitive
    h.assert_eq[BasicPrimitive](p2, p3)
    h.assert_true(#(p2 == p3))

class iso _TestCompileTimeString is UnitTest

  fun name(): String => "CompileTimeExpression/CompileTimeString"

  fun apply(h: TestHelper) =>
    let s1: String = # "Hello"
    h.assert_eq[String](s1, "Hello")
    let s2: String = # " World"
    h.assert_eq[String](s2, " World")
