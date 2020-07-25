//
// Created by xc5 on 2020/7/24.
//

#ifndef OCC_CG_VARIANT_H
#define OCC_CG_VARIANT_H


/* Type used to hold a variant:
 */
typedef enum {

V_NONE  =		0,	/* default empty variant */

/* ====================================================================
 *
 * Conditions for conditional branch operators
 *
 * ====================================================================
 */

V_BR_NONE = 	0,	/* No branch variant */

V_BR_I8EQ0 = 	1,	/* Signed integer A = 0 */
V_BR_I8NE0 = 	2,	/* Signed integer A != 0 */
V_BR_I8GT0 = 	3,	/* Signed integer A > 0 */
V_BR_I8GE0 = 	4,	/* Signed integer A >= 0 */
V_BR_I8LT0 = 	5,	/* Signed integer A < 0 */
V_BR_I8LE0 = 	6,	/* Signed integer A <= 0 */

V_BR_I8EQ = 	7,	/* Signed integer A = B */
V_BR_I8NE = 	8,	/* Signed integer A != B */
V_BR_I8GT = 	9,	/* Signed integer A > B */
V_BR_I8GE = 	10,	/* Signed integer A >= B */
V_BR_I8LT = 	11,	/* Signed integer A < B */
V_BR_I8LE = 	12,	/* Signed integer A <= B */

V_BR_U8EQ0 = 	13,	/* Unsigned integer A = 0 */
V_BR_U8NE0 = 	14,	/* Unsigned integer A != 0 */
V_BR_U8GT0 = 	15,	/* Unsigned integer A > 0 */
V_BR_U8GE0 = 	16,	/* Unsigned integer A >= 0 */
V_BR_U8LT0 = 	17,	/* Unsigned integer A < 0 */
V_BR_U8LE0 = 	18,	/* Unsigned integer A <= 0 */

V_BR_U8EQ = 	19,	/* Unsigned integer A = B */
V_BR_U8NE = 	20,	/* Unsigned integer A != B */
V_BR_U8GT = 	21,	/* Unsigned integer A > B */
V_BR_U8GE = 	22,	/* Unsigned integer A >= B */
V_BR_U8LT = 	23,	/* Unsigned integer A < B */
V_BR_U8LE = 	24,	/* Unsigned integer A <= B */

V_BR_FEQ = 	31,	/* Floating point A = B */
V_BR_FNE = 	32,	/* Floating point A != B */
V_BR_FGT = 	33,	/* Floating point A > B */
V_BR_FGE = 	34,	/* Floating point A >= B */
V_BR_FLT = 	35,	/* Floating point A < B */
V_BR_FLE = 	36,	/* Floating point A <= B */

V_BR_FOR = 	37,	/* Floating point ordered compare */
V_BR_FUO = 	38,	/* Floating point unordered compare */
V_BR_DOR = 	39,	/* Double floating ordered compare */
V_BR_DUO = 	40,	/* Double floating unordered compare */

V_BR_DEQ = 	43,	/* Double floating point A = B */
V_BR_DNE = 	44,	/* Double floating point A != B */
V_BR_DGT = 	45,	/* Double floating point A > B */
V_BR_DGE = 	46,	/* Double floating point A >= B */
V_BR_DLT = 	47,	/* Double floating point A < B */
V_BR_DLE = 	48,	/* Double floating point A <= B */

V_BR_QEQ = 	49,	/* Quad floating point A = B */
V_BR_QNE = 	50,	/* Quad floating point A != B */
V_BR_QGT = 	51,	/* Quad floating point A > B */
V_BR_QGE = 	52,	/* Quad floating point A >= B */
V_BR_QLT = 	53,	/* Quad floating point A < B */
V_BR_QLE = 	54,	/* Quad floating point A <= B */


V_BR_I4EQ0 = 	55,	/* Signed integer A = 0 */
V_BR_I4NE0 = 	56,	/* Signed integer A != 0 */
V_BR_I4GT0 = 	57,	/* Signed integer A > 0 */
V_BR_I4GE0 = 	58,	/* Signed integer A >= 0 */
V_BR_I4LT0 = 	59,	/* Signed integer A < 0 */
V_BR_I4LE0 = 	60,	/* Signed integer A <= 0 */

V_BR_I4EQ = 	61,	/* 4-byte signed integer A = B */
V_BR_I4NE = 	62,	/* 4-byte signed integer A != B */
V_BR_I4GT = 	63,	/* 4-byte signed integer A > B */
V_BR_I4GE = 	64,	/* 4-byte signed integer A >= B */
V_BR_I4LT = 	65,	/* 4-byte signed integer A < B */
V_BR_I4LE = 	66,	/* 4-byte signed integer A <= B */

V_BR_U4EQ0 = 	67,	/* Unsigned integer A = 0 */
V_BR_U4NE0 = 	68,	/* Unsigned integer A != 0 */
V_BR_U4GT0 = 	69,	/* Unsigned integer A > 0 */
V_BR_U4GE0 = 	70,	/* Unsigned integer A >= 0 */
V_BR_U4LT0 = 	71,	/* Unsigned integer A < 0 */
V_BR_U4LE0 = 	72,	/* Unsigned integer A <= 0 */

V_BR_U4EQ = 	73,	/* 4-byte unsigned integer A = B */
V_BR_U4NE = 	74,	/* 4-byte unsigned integer A != B */
V_BR_U4GT = 	75,	/* 4-byte unsigned integer A > B */
V_BR_U4GE = 	76,	/* 4-byte unsigned integer A >= B */
V_BR_U4LT = 	77,	/* 4-byte unsigned integer A < B */
V_BR_U4LE = 	78,	/* 4-byte unsigned integer A <= B */

V_BR_F_FALSE = 	79,	/* Floating point (fcc) false */
V_BR_F_TRUE = 	80,	/* Floating point (fcc) true */

V_BR_P_TRUE = 	81,	/* Predicate true */
V_BR_PEQ = 	82,	/* Predicate A = B */
V_BR_PNE = 	83,	/* Predicate A != B */

V_BR_CLOOP= 84,	/* Counted loop */
V_BR_CTOP = 85,	/* Mod-sched counted loop (top) */
V_BR_CEXIT= 86,	/* Mod-sched counted loop (exit) */
V_BR_WTOP = 87,	/* Mod-sched while loop (top) */
V_BR_WEXIT= 88,	/* Mod-sched while loop (exit) */

V_BR_XEQ = 89,      /* Double extended  A = B */
V_BR_XNE = 90,      /* Double extended  A != B */
V_BR_XGT = 91,      /* Double extended  A > B */
V_BR_XGE = 92,      /* Double extended  A >= B */
V_BR_XLT = 93,      /* Double extended  A < B */
V_BR_XLE = 94,      /* Double extended  A <= B */

V_BR_ALWAYS = 	95,	/* Unconditional branch */
V_BR_NEVER = 	96,	/* Never branch */
V_BR_LAST = 	97,	/* Last one defined, must < 128 */

} VARIANT;

/* V_BR_MASK *must* be 2^n - 1, and be at least as large as  */
/* V_BR_LAST */
#define V_BR_MASK	0x7f	/* Mask for branch condition */
#define V_br_condition(v)	((v) & V_BR_MASK)

/*
 * Store whether doing true or false branch in the variant,
 * separate from the branch condition.
 * True-branch is the default, 0 value.
 */
#define V_BR_FALSE	0x0080	/* do false branch rather than true branch */

#define V_false_br(v)		((v) & V_BR_FALSE)
#define Set_V_false_br(v)	((v) |= V_BR_FALSE)
#define Set_V_true_br(v)	((v) &= ~V_BR_FALSE)


#endif //OCC_CG_VARIANT_H
