#include <ctype.h>
#include <math.h>

#include "math_elem.h"
#include "lib/bintree.h"



MathElem scanMathElem (FILE* file, char c, char* buffer){
    MathElem ret = {};
    if (isdigit(c)){
        ret.type = MATH_CONST;
        char c1 = fgetc(file);
        ungetc(c1, file);
        fscanf(file, "%lg", &(ret.val));
        return ret;
    }

    bool op_char = strchr(OP_ONLY_CHARS, c) != nullptr;
    if (op_char)
        fscanf(file, "%[" OP_ONLY_CHARS "]", buffer);
    else
        fscanf(file, "%[^" OP_ONLY_CHARS CNTRL_CHARS SPACE_CHARS"]", buffer);

    if (*buffer == '\0'){
        ret.type = MATH_PAIN;
        return ret;
    }

    mathOpType_t op_type = scanMathOp(buffer);
    if (op_type != MATH_O_NOTOP){
        ret.type = MATH_OP;
        ret.op   = op_type;
        return ret;
    }
    if (op_char){
        ret.type = MATH_PAIN;
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
    case MATH_FUNC:
        fprintf(file, "%s", elem.name);
        break;
    case MATH_CNTRL:
        fprintf(file, "$%c", elem.chr);
        break;
    default:
        fprintf(file, "TRASH");
        break;
    }
}
