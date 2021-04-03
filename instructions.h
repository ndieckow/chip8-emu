#include "chip8.h"

// Instructions
void SUB_clear_return(Chip8* chip8);
void jump(Chip8 *chip8);
void call(Chip8 *chip8);
void skip_eq(Chip8 *chip8);
void skip_neq(Chip8 *chip8);
void skip_eq_reg(Chip8 *chip8);
void load_reg(Chip8 *chip8);
void add_const(Chip8 *chip8);
void SUB_arithmetic(Chip8* chip8);
void skip_neq_reg(Chip8 *chip8);
void load_I(Chip8 *chip8);
void jump_offset(Chip8 *chip8);
void random_and(Chip8 *chip8);
void draw(Chip8 *chip8);
void SUB_skip(Chip8 *chip8);
void SUB_load_add(Chip8 *chip8);

// SUB 8 (arithmetic)
void copy(Chip8 *chip8);
void bit_or(Chip8 *chip8);
void bit_and(Chip8 *chip8);
void bit_xor(Chip8 *chip8);
void add_reg(Chip8 *chip8);
void sub_reg(Chip8 *chip8);
void shift_right(Chip8 *chip8);
void sub_reg_flip(Chip8 *chip8);
void shift_left(Chip8 *chip8);

// Array of function pointers to instructions
void (*call_instruction[16]) (Chip8 *chip8) = {
    SUB_clear_return, jump, call, skip_eq, skip_neq, skip_eq_reg, load_reg,
    add_const, SUB_arithmetic, skip_neq_reg, load_I, jump_offset, random_and,
    draw, SUB_skip, SUB_load_add
};
void (*call_sub_8[8]) (Chip8 *chip8) = {
    copy, bit_or, bit_and, bit_xor,
    add_reg, sub_reg, shift_right, sub_reg_flip
}; // 8xxE is extra
