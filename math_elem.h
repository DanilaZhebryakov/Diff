#ifndef MATH_ELEM_H_INCLUDED
#define MATH_ELEM_H_INCLUDED
#include <stdio.h>

enum mathDataType_t{
    MATH_CONST = 1,
    MATH_VAR   = 2,
    MATH_OP    = 3,
    MATH_PAIN  = 0xFF
};

enum mathOpType_t{
    MATH_O_NOTOP = 0,
    MATH_O_ADD   = 1,
    MATH_O_SUB   = 2,
    MATH_O_MUL   = 3,
    MATH_O_DIV   = 4,
    MATH_O_POW   = 5,
    MATH_O_LOG   = 6
};


struct MathElem{
    mathDataType_t type;
    union {double val; char* name; mathOpType_t op;};
};

#define BAD_MATH_DATA {MATH_PAIN}
const int MAX_FORM_WORD_LEN = 1000;

mathOpType_t scanMathOp(const char* buffer);

MathElem scanMathElem (FILE* file, char c, char* buffer);

void printMathElem(FILE* file, MathElem elem);

#endif
