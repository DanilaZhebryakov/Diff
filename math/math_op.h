#ifndef MATH_OP_H_INCLUDED
#define MATH_OP_H_INCLUDED


#define MATH_OP_DEF(_enum, _enumval, _name, _pri, _ret) \
_enum = _enumval,

enum mathOpType_t{
    MATH_O_UNARY = 0x80,
    MATH_O_NOTOP = 0,
    #include "math_op_defines.h"
};
#undef MATH_OP_DEF

const int MAX_MATH_OP_PRIORITY = 5;

bool isMathOpUnary(mathOpType_t op);

bool canMathOpBeUnary(mathOpType_t op);

int getMathOpPriority(mathOpType_t op_type);

const char* mathOpName(mathOpType_t op_type);

mathOpType_t scanMathOp(const char* buffer);

double calcMathOp(mathOpType_t op_type, double a, double b);

#endif // MATH_OP_H_INCLUDED
