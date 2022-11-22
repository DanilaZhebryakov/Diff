#include <ctype.h>
#include <math.h>

#include "math_elem.h"
#include "lib/bintree.h"


struct MathOp{
    const char* name;
    mathOpType_t op;
    int priorirty;
};

static const MathOp oplist[] = {
    {"+"    , MATH_O_ADD  , 0},
    {"-"    , MATH_O_SUB  , 0},
    {"*"    , MATH_O_MUL  , 1},
    {"/"    , MATH_O_DIV  , 1},
    {"^"    , MATH_O_POW  , 2},
    {"log"  , MATH_O_LOG  , 100},
    {"exp"  , MATH_O_EXP  , 100},
    {"ln"   , MATH_O_LN   , 100},
    {"sin"  , MATH_O_SIN  , 100},
    {"cos"  , MATH_O_COS  , 100},
    {"tg"   , MATH_O_TG   , 100},
    {"asin" , MATH_O_ASIN , 100},
    {"acos" , MATH_O_ACOS , 100},
    {"atg"  , MATH_O_ATG  , 100},
    {"sh"   , MATH_O_SH   , 100},
    {"ch"   , MATH_O_CH   , 100},
    {"th"   , MATH_O_TH   , 100},
    {"sqrt" , MATH_O_SQRT , 100},
    {"o"    , MATH_O_o    , 100},
};

bool isMathOpUnary(mathOpType_t op){
    return op & MATH_O_UNARY;
}

static const int opcount = sizeof(oplist) / sizeof(MathOp);

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

double calcMathOp(mathOpType_t op_type, double a, double b){
    switch (op_type){
    case MATH_O_ADD:
        return a + b;
    case MATH_O_SUB:
        return a - b;
    case MATH_O_MUL:
        return a * b;
    case MATH_O_DIV:
        return a / b;
    case MATH_O_POW:
        return pow(a,b);
    case MATH_O_LOG:
        return log(b) / log(a);
    case MATH_O_EXP:
        return exp(b);
    case MATH_O_LN:
        return log(b);
    case MATH_O_SIN:
        return sin(b);
    case MATH_O_COS:
        return cos(b);
    case MATH_O_TG:
        return tan(b);
    case MATH_O_ASIN:
        return asin(b);
    case MATH_O_ACOS:
        return acos(b);
    case MATH_O_ATG:
        return atan(b);
    case MATH_O_SH:
        return sinh(b);
    case MATH_O_CH:
        return cosh(b);
    case MATH_O_TH:
        return tanh(b);
    case MATH_O_SQRT:
        return sqrt(b);
    case MATH_O_o:
        return 0;
    default:
        error_log("BADOP\n");
        return NAN;
    }
}

mathOpType_t scanMathOp(const char* buffer){
    for (int i = 0; i < opcount; i++){
        if (strcmp(oplist[i].name, buffer) == 0){
            return oplist[i].op;
        }
    }
    return MATH_O_NOTOP;
}

MathElem scanMathElem (FILE* file, char c, char* buffer){
    MathElem ret = {};
    if (isdigit(c)){
        ret.type = MATH_CONST;
        char c1 = fgetc(file);
        ungetc(c1, file);
        fscanf(file, "%lg", &(ret.val));
        return ret;
    }
    fscanf(file, "%[^ ()0-9]", buffer);
    if(*buffer == '\0'){
        ret.type = MATH_PAIN;
        return ret;
    }

    mathOpType_t op_type = scanMathOp(buffer);
    if (op_type != MATH_O_NOTOP){
        ret.type = MATH_OP;
        ret.op   = op_type;
        return ret;
    }

    ret.type = MATH_VAR;
    ret.name = strdup(buffer);
    return ret;
}

void printMathElem(FILE* file, MathElem elem){
    switch (elem.type){
    case MATH_PAIN:
        fprintf(file, "PAIN");
        break;
    case MATH_OP:
        fprintf(file, mathOpName(elem.op));
        break;
    case MATH_CONST:
        fprintf(file, "%lg", elem.val);
        break;
    case MATH_VAR:
        fprintf(file, "%s", elem.name);
        break;
    default:
        fprintf(file, "TRASH");
        break;
    }
}
