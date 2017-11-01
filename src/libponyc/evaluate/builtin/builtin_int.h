#ifndef BUILTIN_INT_H
#define BUILTIN_INT_H

#include "../../ast/ast.h"
#include "../../pass/pass.h"

bool int_create(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_min_value(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_max_value(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

bool int_add(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_sub(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_mul(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_div(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

bool int_neg(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_eq(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_ne(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_lt(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_le(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_ge(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_gt(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

bool int_min(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_max(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

bool int_and(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_or(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_xor(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_not(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_shl(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_shr(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

// Casting methods(pass_opt_t* opt, bool receiver, ast_t* args, ast_t** result);
bool int_i8(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_i16(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_i32(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_i64(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_i128(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_ilong(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_isize(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_u8(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_u16(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_u32(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_u64(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_u128(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_ulong(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_usize(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

bool int_f32(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);
bool int_f64(pass_opt_t* opt, ast_t* receiver, ast_t* args, ast_t** result);

#endif
