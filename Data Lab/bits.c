#include <stdio.h>




/* 
 * CS:APP Data Lab 
 * 
 * <Joshua Liu     105136031>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */
/* We do not support C11 <threads.h>.  */
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
/* This function uses DeMorgan's Law to make the
 *   | and ~ operators work as an &. We not x and y,
 *   then or them, and not that result
 */
  return ~(~x | ~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (least significant) to 3 (most significant)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
/* We shift x to the right by n*8 bits
 *   to result with the correct 8 bits on the
 *   right. Then, we & it with 0xFF to get the
 *   8 bits we want.
 */
  int byte = x >> (n<<3);
  return (byte & 0xFF);
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
/* We find perform an arithmetic shift first, then have to get rid
 *   of extra bits. We create a mask, then we & the two together
 *   to get our desired outcome.
 */
  int shift = x >> n;
  int mask = (0xFF << 8) + 0xFF;
  mask = (mask << 16) + mask; // all 1's
  mask = mask << (31+(~n+1)); // shift by 31-n times;
  mask = mask << 1; // shift one since we cannot shift by 32 times if n is 0
  mask = ~mask; // ~ to get the mask
  return (mask & shift);
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
/* We create a mask with 0001 0001 ... 0001 0001. Then, we do (x & mask) four times, each
 *   time shifting x to the right by 1 excluding the first time. This creates a number with
 *   the total bit count in every four bits, or every hex digit. We & these digits with 0xF and
 *   add the count up, resulting with the total count.
 */  
  int mask = 0x11;
  int num = 0;
  int count1 = 0;
  int count2 = 0;
  mask = (mask << 8) + mask;
  mask = (mask << 16) + mask; // 0001 0001 ... 0001 0001
  num = (x & mask) + ((x >> 1) & mask) + ((x >> 2) & mask) + ((x >> 3) & mask);
  count1 = (num & 0xF) + (num >> 4 & 0xF) + (num >> 8 & 0xF) + (num >> 12 & 0xF); // first 16 bits
  count2 = (num >> 16 & 0xF) + (num >> 20 & 0xF) + (num >> 24 & 0xF) + (num >> 28 & 0xF); // last 16 bits
  return (count1 + count2);
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
/* We extend the number so that there will be a 1 in the LSB if there is a 1
 *   anywhere in the number. Then, we take that last bit (either 1 or 0) and
 *   flip it, extract the last bit, and return the value.
 */
  int num = x | (x >> 1);
  num = num | (num >> 2);
  num = num | (num >> 4);
  num = num | (num >> 8);
  num = num | (num >> 16); // now there is a 1 in LSB if there is 1 in number
  return (~num & 1);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
/* We first determine whether x is true or false with the ! operator to return
 *   0 or 1, then create a mask where 111... is true and 000... is false.
 *   Then, we & these with y and z and then add them to get the result.
 */ 
  int cond = !!x;
  int mask1 = cond << 31;
  int mask2 = 0;
  mask1 = mask1 >> 31; // 111... for true, 000... for false
  mask2 = ~mask1;
  y = y & mask1;
  z = z & mask2;
  return (y + z);
}
/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int isPower2(int x) {
/* We use the idea that if (x & (x - 1)) == 0, then x is a power of two,
 *   but when (x & (x - 1)) == 1, then x is not a power of two. We then
 *   also check if x is negative, as well as if it is 0.
 */
  int noMSB = x << 1;
  int pow2 = noMSB & (noMSB + (~1 + 1)); // if isPower2, then num = 0, else num = 1
  return !pow2 & !((x >> 31) & 1) & !!x;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
/* We create a 0xAAAAAAAA mask so that when we & x with the mask,
 *   we should get the mask back. We check this with ^, then a !.
 */
  int mask = 0xAA;
  mask = (mask << 8) + mask; // = 0xAAAA
  mask = (mask << 16) + mask; // = 0xAAAAAAAA
  return !((x & mask) ^ mask);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
/* We shift x by n - 1 bits so that the signed bit remains. If the number
 *   fits in n-bits, then the remaining bits should either be all 1
 *   or all 0, which we then check for.
 */
  int shift = x >> (n + ~0); // shifts by n-1, leaving signed bit
  int all1 = !(shift + 1);
  int all0 = !shift;
  return (all1 ^ all0);
}
/* 
 * dividePower2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: dividePower2(15,1) = 7, dividePower2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int dividePower2(int x, int n) {
/* If x is negative, we have to add to it a bias value before right shifting.
 *   We find the bias value, determine the sign, then add it accordingly, using
 *   the & operator with a sign mask.
 */
  int biasVal = (1 << n) + ~0;
  int sign = x >> 31;
  int bias = biasVal & sign;
  return (x + bias) >> n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
/* This is solved by finding the negative of a number and returning it,
 *   which is simply flipping the bits and adding 1.
 */
  return (~x + 1);
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 2
 */
int isPositive(int x) {
/* We right shift to get a 32-bit number with all 1's when x is negative and all 0's
 *   when x is positive. Then, we use ! to return the correct value, 1 or 0. We also
 *   need to take into account x = 0, which should return 0, solved with !!x.
 */
  int sign = x >> 31;
  return !sign & !!x;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
/* We determine the following: function returns true if:
 *   (x is min),
 *   (x and y different signs & x is negative),
 *   (x and y same signs & difference is positive)
 *   Using these conditionals, we determine if x isLessOrEqual to y.
 */
  int sign_x = (x >> 31) & 1; // 0 if positive, 1 if negative
  int sign_y = (y >> 31) & 1; // 0 if positive, 1 if negative
  int sign_eval = !(sign_x ^ sign_y); // returns 0 if different signs, else returns 1
  int diff = y + (~x + 1); // finds difference
  int sign_diff = (diff >> 31) & 1; // 0 if positive, 1 if negative
  int x_is_min = !(x ^ (1 << 31)); // 1 if minimum, 0 if not
  return (x_is_min) | ((!sign_eval) & sign_x) | (sign_eval & (!sign_diff));
  // true if: (x is min), (x and y different signs & x is negative), and (x and y same signs & difference is positive) 
}
/*
 * intLog2 - return floor(log base 2 of x), where x > 0
 *   Example: intLog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int intLog2(int x) {
/* We first make all the bits to the right of the largest 1 bit all 1's. Since
 *   log_2(x) is (the number of 1's - 1), we use bitCount function's logic to
 *   count the number of 1 bits. This takes the number, masks it to 0001 0001 ... 0001 0001
 *   for all the bits(by doing it 4 times, shifting number to the right by 1 every time,
 *   and adding the count together. Then, we subtract by 1. This yields the
 *   floor(log base 2 of x).
 */
  int num = x | (x >> 1);
  int mask = 0x11;
  int count1 = 0;
  int count2 = 0;
  int totalCount = 0;
  num = num | (num >> 2);
  num = num | (num >> 4);
  num = num | (num >> 8);
  num = num | (num >> 16); // now there is a 1 in LSB if there is 1 in number
  mask = (mask << 8) + mask;
  mask = (mask << 16) + mask; // 0001 0001 ... 0001 0001
  num = (num & mask) + ((num >> 1) & mask) + ((num >> 2) & mask) + ((num >> 3) & mask);
  count1 = (num & 0xF) + (num >> 4 & 0xF) + (num >> 8 & 0xF) + (num >> 12 & 0xF); // first 16 bits
  count2 = (num >> 16 & 0xF) + (num >> 20 & 0xF) + (num >> 24 & 0xF) + (num >> 28 & 0xF); // last 16 bits
  totalCount = count1 + count2 + (~0);
  return totalCount;
}
/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2 
 */
int leastBitPos(int x) {
/* We notice that using & between x and -x, (x & -x) results with a mask
 *   of the least significant bit. We change the -x to (~x + 1).
 */
  return (x & (~x + 1));
}




int main()
{
	printf("%d", isLessOrEqual(-4, 5));
}