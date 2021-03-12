/* NN.H - header file for NN.C */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security, Inc., created 1991. All rights reserved. */

/* Type definitions. */

typedef UINT4 NN_DIGIT;
typedef UINT2 NN_HALF_DIGIT;

/* Constants.

   Note: MAX_NN_DIGITS is long enough to hold any RSA modulus, plus
   one more digit as required by R_GeneratePEMKeys (for n and phiN,
   whose lengths must be even). All natural numbers have at most
   MAX_NN_DIGITS digits, except for double-length intermediate values
   in NN_Mult (t), NN_ModMult (t), NN_ModInv (w), and NN_Div (c).
 */

/* Length of digit in bits */
#define NN_DIGIT_BITS 32
#define NN_HALF_DIGIT_BITS 16
/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS / 8)
/* Maximum length in digits */
#define MAX_NN_DIGITS \
  ((MAX_RSA_MODULUS_LEN + NN_DIGIT_LEN - 1) / NN_DIGIT_LEN + 1)
/* Maximum digits */
#define MAX_NN_DIGIT 0xffffffff
#define MAX_NN_HALF_DIGIT 0xffff

/* Macros.
 */
#define LOW_HALF(x) ((x) & MAX_NN_HALF_DIGIT)
#define HIGH_HALF(x) (((x) >> NN_HALF_DIGIT_BITS) & MAX_NN_HALF_DIGIT)
#define TO_HIGH_HALF(x) (((NN_DIGIT)(x)) << NN_HALF_DIGIT_BITS)
#define DIGIT_MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 1)) & 1)
#define DIGIT_2MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 2)) & 3)

void NN_Decode PROTO_LIST((NN_DIGIT *, unsigned int, unsigned char *, unsigned int));
void NN_Encode PROTO_LIST((unsigned char *, unsigned int, NN_DIGIT *, unsigned int));

void NN_Assign PROTO_LIST ((NN_DIGIT *, NN_DIGIT *, unsigned int));
void NN_AssignZero PROTO_LIST ((NN_DIGIT *, unsigned int));
void NN_Assign2Exp PROTO_LIST ((NN_DIGIT *, unsigned int, unsigned int));

NN_DIGIT NN_Add PROTO_LIST((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int));
NN_DIGIT NN_Sub PROTO_LIST((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int));
void NN_Mult PROTO_LIST ((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int));
void NN_Div PROTO_LIST((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int));
NN_DIGIT NN_LShift PROTO_LIST((NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int));
NN_DIGIT NN_RShift PROTO_LIST((NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int));

void NN_Mod PROTO_LIST((NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int));
void NN_ModMult PROTO_LIST((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int));
void NN_ModExp PROTO_LIST((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int));
void NN_ModInv PROTO_LIST((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int));
void NN_Gcd PROTO_LIST ((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int));

int NN_Cmp PROTO_LIST ((NN_DIGIT *, NN_DIGIT *, unsigned int));
int NN_Zero PROTO_LIST ((NN_DIGIT *, unsigned int));
unsigned int NN_Bits PROTO_LIST ((NN_DIGIT *, unsigned int));
unsigned int NN_Digits PROTO_LIST ((NN_DIGIT *, unsigned int));

#define NN_ASSIGN_DIGIT(a, b, digits) {NN_AssignZero (a, digits); a[0] = b;}
#define NN_EQUAL(a, b, digits) (! NN_Cmp (a, b, digits))
#define NN_EVEN(a, digits) (((digits) == 0) || ! (a[0] & 1))
