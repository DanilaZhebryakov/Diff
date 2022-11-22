#ifndef MATH_ELEM_H_INCLUDED
#define MATH_ELEM_H_INCLUDED
#include <stdio.h>

enum mathDataType_t{
    MATH_CONST = 1,
    MATH_VAR   = 2,
    MATH_OP    = 3,
    MATH_FUNC  = 4,
    MATH_PAIN  = 0xFF
};

enum mathOpType_t{
    MATH_O_UNARY = 0x80,

    MATH_O_NOTOP = 0,
    MATH_O_ADD   = 1,
    MATH_O_SUB   = 2,
    MATH_O_MUL   = 3,
    MATH_O_DIV   = 4,
    MATH_O_POW   = 5,
    MATH_O_LOG   = 6,
    MATH_O_EXP   = 7  | 0x80,
    MATH_O_LN    = 8  | 0x80,
    MATH_O_SIN   = 9  | 0x80,
    MATH_O_COS   = 10 | 0x80,
    MATH_O_TG    = 11 | 0x80,
    MATH_O_ASIN  = 12 | 0x80,
    MATH_O_ACOS  = 13 | 0x80,
    MATH_O_ATG   = 14 | 0x80,
    MATH_O_SH    = 15 | 0x80,
    MATH_O_CH    = 16 | 0x80,
    MATH_O_TH    = 17 | 0x80,
    MATH_O_SQRT  = 18 | 0x80,
    MATH_O_o     = 19 | 0x80,
};

const double math_eps = 0.0001;

struct MathElem{
    mathDataType_t type;
    union {double val; char* name; mathOpType_t op;};
};

#define BAD_MATH_DATA {MATH_PAIN}
const int MAX_FORM_WORD_LEN = 1000;

bool isMathOpUnary(mathOpType_t op);

int getMathOpPriority(mathOpType_t op_type);

const char* mathOpName(mathOpType_t op_type);

mathOpType_t scanMathOp(const char* buffer);

double calcMathOp(mathOpType_t op_type, double a, double b);

MathElem scanMathElem (FILE* file, char c, char* buffer);

void printMathElem(FILE* file, MathElem elem);


#endif
