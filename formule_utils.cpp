#include "formule_utils.h"
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

MathData scanMathElem (FILE* file, char c, char* buffer){
    MathData ret = {};
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

void printMathElem(FILE* file, MathData elem){
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

bool scanMathForm_(FILE* file, BinTreeNode** tree_place, char* buffer, bool full_scan = false){
    assert_log(tree_place != nullptr);
    do{
        char c = ' ';
        while(isspace(c)){
            c = fgetc(file);
        }
        if(c == EOF || c == ')'){
            return true;
        }
        if(c == '('){
            scanMathForm_(file, tree_place, buffer, true);
            continue;
        }

        ungetc(c, file);
        MathData data = scanMathElem(file, c, buffer);
        BinTreeNode* old_node = *tree_place;
        BinTreeNode* new_node = binTreeNewNode(data);
        *tree_place = new_node;
        new_node->left = old_node;
        if(data.type == MATH_OP){
            scanMathForm_(file, &(new_node->right), buffer);
            if(!old_node){
                error_log("no arg");
                return false; //TODO: add unary check
            }
            binTreeUpdSize(new_node);
        }
        else{
            if(old_node){
                error_log("excess arg");
                return false;
            }
        }
    } while(full_scan);
    return true;
}

BinTreeNode* scanMathForm(FILE* file){
    char* buffer = (char*)calloc(MAX_FORM_WORD_LEN, sizeof(char));
    BinTreeNode* tree =  nullptr;
    bool res = scanMathForm_(file, &tree, buffer);
    free(buffer);
    if (!res){
        binTreeDump(tree);
        binTreeDtor(tree);
        return nullptr;
    }
    return tree;
}

void printMathForm(FILE* file, BinTreeNode* form){
    if (!form)
        return;
    if (form->data.type == MATH_OP){
        fprintf(file, "(");
        printMathForm(file, form->left);
        fprintf(file, " ");
        printMathElem(file, form->data);
        fprintf(file, " ");
        printMathForm(file, form->right);
        fprintf(file, ")");
    }
    else{
        printMathElem(file, form->data);
    }

}
