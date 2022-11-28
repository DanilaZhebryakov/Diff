#include <math.h>
#include <string.h>

#include "math_op.h"

struct MathOp{
    const char* name;
    mathOpType_t op;
    int priorirty;
};

#define MATH_OP_DEF(_enum, _enumval, _name, _pri, _ret) \
{_name, _enum, _pri},

static const MathOp oplist[] = {
    #include "math_op_defines.h"
};
#undef MATH_OP_DEF

bool isMathOpUnary(mathOpType_t op){
    return op & MATH_O_UNARY;
}

static const int opcount = sizeof(oplist) / sizeof(MathOp);

bool canMathOpBeUnary(mathOpType_t op){
    for (int i = 0; i < opcount; i++){
        if (oplist[i].op == (op | MATH_O_UNARY)){
            return true;
        }
    }
    return false;
}

int getMathOpPriority(mathOpType_t op_type){
    for (int i = 0; i < opcount; i++){
        if (oplist[i].op == op_type){
            return oplist[i].priorirty;
        }
    }
    return -1;
}

const char* mathOpName(mathOpType_t op_type){
    for (int i = 0; i < opcount; i++){
        if (oplist[i].op == op_type){
            return oplist[i].name;
        }
    }
    return "BADOP";
}

#define MATH_OP_DEF(_enum, _enumval, _name, _pri, _ret) \
case _enum:\
    return _ret;

double calcMathOp(mathOpType_t op_type, double a, double b){
    switch (op_type){
    #include "math_op_defines.h"
    default:
        return NAN;
    }
}
#undef MATH_OP_DEF


mathOpType_t scanMathOp(const char* buffer){
    for (int i = 0; i < opcount; i++){
        if (strcmp(oplist[i].name, buffer) == 0){
            return oplist[i].op;
        }
    }
    return MATH_O_NOTOP;
}
