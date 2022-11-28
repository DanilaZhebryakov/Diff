#ifndef MATH_ELEM_H_INCLUDED
#define MATH_ELEM_H_INCLUDED
#include <stdio.h>
#include "math_op.h"

enum mathDataType_t{
    MATH_CONST = 1,
    MATH_VAR   = 2,
    MATH_OP    = 3,
    MATH_FUNC  = 4,
    MATH_CNTRL = 5,
    MATH_PAIN  = 0xFF
};

#define OP_ONLY_CHARS "-+%*/\\^&|%!?~,"
#define CNTRL_CHARS "(){};"

const double math_eps = 0.0001;

struct MathElem{
    mathDataType_t type;
    union {double val; char* name; mathOpType_t op; char chr;};
};

#define BAD_MATH_DATA {MATH_PAIN}
const int MAX_FORM_WORD_LEN = 1000;

MathElem scanMathElem (FILE* file, char c, char* buffer);

void printMathElem(FILE* file, MathElem elem);


#endif
