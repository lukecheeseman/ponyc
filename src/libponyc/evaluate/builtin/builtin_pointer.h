#ifndef BUILTIN_POINTER_H
#define BUILTIN_POINTER_H

#include "../../pass/pass.h"

void builtin_pointer_add_methods(pass_opt_t* opt);

typedef struct pointer pointer_t;

uint64_t ptr_get_size(pointer_t* ptr);

ast_t** ptr_get_elements(pointer_t* ptr);

/*
  fun tag _unsafe(): Pointer[A] ref =>
    """
    Unsafe change in reference capability.
    """
    compile_intrinsic

  fun _offset(n: USize): this->Pointer[A] =>
    """
    Return a pointer to the n-th element.
    """
    compile_intrinsic

  fun tag _element_size(): USize =>
    """
    Return the size of a single element in an array of type A.
    """
    compile_intrinsic

  fun ref _insert(n: USize, len: USize): Pointer[A] =>
    """
    Creates space for n new elements at the head, moving following elements.
    The array length before this should be len, and the available space should
    be at least n + len.
    """
    compile_intrinsic

  fun ref _delete(n: USize, len: USize): A^ =>
    """
    Delete n elements from the head of pointer, compact remaining elements of
    the underlying array. The array length before this should be n + len.
    Returns the first deleted element.
    """
    compile_intrinsic

  fun _copy_to(that: Pointer[this->A!], n: USize): this->Pointer[A] =>
    """
    Copy n elements from this to that.
    """
    compile_intrinsic

  fun ref _consume_from(that: Pointer[A]^, n: USize): Pointer[A]^ =>
    """
    Copy n elements from that to this.
    """
    compile_intrinsic

  fun tag usize(): USize =>
    """
    Convert the pointer into an integer.
    """
    compile_intrinsic

  fun tag is_null(): Bool =>
    """
    Return true for a null pointer, false for anything else.
    """
    compile_intrinsic

  fun tag eq(that: Pointer[A] tag): Bool =>
    """
    Return true if this address is that address.
    """
    compile_intrinsic

  fun tag lt(that: Pointer[A] tag): Bool =>
    """
    Return true if this address is less than that address.
    """
    compile_intrinsic

  fun tag ne(that: Pointer[A] tag): Bool => not eq(that)
  fun tag le(that: Pointer[A] tag): Bool => lt(that) or eq(that)
  fun tag ge(that: Pointer[A] tag): Bool => not lt(that)
  fun tag gt(that: Pointer[A] tag): Bool => not le(that)

  fun tag hash(): U64 =>
    """
    Returns a hash of the address.
    """
    usize().hash()
*/
#endif
