#include <ctype.h>
#include <math.h>

#include "math_elem.h"
#include "lib/bintree.h"


struct MathOp{
    const char* name;
    mathOpType_t op;
};

static const MathOp oplist[] = {
    {"+"    , MATH_O_ADD  },
    {"-"    , MATH_O_SUB  },
    {"*"    , MATH_O_MUL  },
    {"/"    , MATH_O_DIV  },
    {"^"    , MATH_O_POW  },
    {"log"  , MATH_O_LOG  },
    {"exp"  , MATH_O_EXP  },
    {"ln"   , MATH_O_LN   },
    {"sin"  , MATH_O_SIN  },
    {"cos"  , MATH_O_COS  },
    {"tg"   , MATH_O_TG   },
    {"asin" , MATH_O_ASIN },
    {"acos" , MATH_O_ACOS },
    {"atg"  , MATH_O_ATG  },
    {"sh"   , MATH_O_SH   },
    {"ch"   , MATH_O_CH   },
    {"th"   , MATH_O_TH   },
    {"sqrt" , MATH_O_SQRT },
    {"o"    , MATH_O_o    },
};

bool isMathOpUnary(mathOpType_t op){
    return op & MATH_O_UNARY;
}

static const int opcount = sizeof(oplist) / sizeof(MathOp);



const char* mathOpName(mathOpType_t op_type){
    for (int i = 0; i < opcount; i++){
        if (oplist[i].op == op_type){
            return oplist[i].name;
        }
    }
    return "";
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
        fscanf(file, "%g", &(ret.val));
        return ret;
    }
    fscanf(file, "%[^ ()0-9]", buffer);

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
        fprintf(file, "%g", elem.val);
        break;
    case MATH_VAR:
        fprintf(file, "%s", elem.name);
        break;
    default:
        fprintf(file, "TRASH");
        break;
    }
}
