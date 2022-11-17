#include "math_elem.h"
#include "lib/bintree.h"
#include "ctype.h"

struct MathOp{
    const char* name;
    mathOpType_t op;
};

static const MathOp oplist[] = {
    {"+"  , MATH_O_ADD},
    {"-"  , MATH_O_SUB},
    {"*"  , MATH_O_MUL},
    {"/"  , MATH_O_DIV},
    {"^"  , MATH_O_POW},
    {"log", MATH_O_LOG}
};
static const int opcount = sizeof(oplist) / sizeof(MathOp);


const char* mathOpName(mathOpType_t op_type){
    for(int i = 0; i < opcount; i++){
        if(oplist[i].op == op_type){
            return oplist[i].name;
        }
    }
    return "";
}

mathOpType_t scanMathOp(const char* buffer){
    for(int i = 0; i < opcount; i++){
        if(strcmp(oplist[i].name, buffer) == 0){
            return oplist[i].op;
        }
    }
    return MATH_O_NOTOP;
}

MathElem scanMathElem (FILE* file, char c, char* buffer){
    MathElem ret = {};
    if (isdigit(c)){
        ret.type = MATH_CONST;
        fscanf(file, "%lf", &(ret.val));
        return ret;
    }
    fscanf(file, "%[^ )0-9]", buffer);

    mathOpType_t op_type = scanMathOp(buffer);
    if(op_type != MATH_O_NOTOP){
        ret.type = MATH_OP;
        ret.op   = op_type;
        return ret;
    }

    ret.type = MATH_VAR;
    ret.name = strdup(buffer);
    return ret;
}

void printMathElem(FILE* file, MathElem elem){
    switch(elem.type){
    case MATH_PAIN:
        fprintf(file, "PAIN");
        break;
    case MATH_OP:
        fprintf(file, mathOpName(elem.op));
        break;
    case MATH_CONST:
        fprintf(file, "%lf", elem.val);
        break;
    case MATH_VAR:
        fprintf(file, "%s", elem.name);
        break;
    default:
        fprintf(file, "TRASH");
        break;
    }
}
