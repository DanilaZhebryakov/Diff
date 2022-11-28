#include "math_elem.h"
#include "formule_utils.h"
#include <ctype.h>

static char getNextMeaningChar(FILE* file){
    char c = ' ';
    do {
        c = fgetc(file);
    } while (isspace(c));
    return c;
}

static void refillElemBuffer_(FILE* file, char* buffer, MathElem* elem_buffer){
    char c = getNextMeaningChar(file);

    if (c == EOF || (strchr(CNTRL_CHARS, c))){
        elem_buffer->type = MATH_CNTRL;
        elem_buffer->chr  = c;
        return;
    }
    ungetc(c, file);

    *elem_buffer = scanMathElem(file, c, buffer);
}

static bool scanMathExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, MathElem* elem_buffer, int priority);

static bool scanMathPExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, MathElem* elem_buffer, bool short_scan = false){
    if (elem_buffer->type != MATH_CNTRL){
        if (elem_buffer->type == MATH_OP){
            if (!canMathOpBeUnary(elem_buffer->op)){
                error_log("Binary operator %s used as unary\n", mathOpName(elem_buffer->op));
                return false;
            }
            elem_buffer->op = (mathOpType_t)(elem_buffer->op | MATH_O_UNARY);
            *tree_place = binTreeNewNode(*elem_buffer);
            BinTreeNode** tree_place_cur = &(*tree_place)->right;

            int op_priority = getMathOpPriority(elem_buffer->op);
            refillElemBuffer_(file, buffer, elem_buffer);

            if (short_scan){
                if (!scanMathPExpr_(file, tree_place_cur, buffer, elem_buffer, true))
                    return false;
            }
            else{
                if (!scanMathExpr_(file, tree_place_cur, buffer, elem_buffer, op_priority))
                   return false;
            }
            binTreeUpdSize(*tree_place);
            return true;

        }
        *tree_place = binTreeNewNode(*elem_buffer);
        if (!short_scan)
            refillElemBuffer_(file, buffer, elem_buffer);

        if ((*tree_place)->data.type == MATH_VAR && (elem_buffer->type == MATH_CNTRL && elem_buffer->chr == '(') && !short_scan){
            (*tree_place)->data.type = MATH_FUNC;
            if (!scanMathPExpr_(file, &((*tree_place)->right), buffer, elem_buffer))
                return false;
            binTreeUpdSize(*tree_place);
        }


        return true;
    }

    char c = elem_buffer->chr;

    if (c == '('){
        refillElemBuffer_(file, buffer, elem_buffer);
        if (!scanMathExpr_(file, tree_place, buffer, elem_buffer, 0))
            return false;
        c = elem_buffer->chr;
        if (c != ')'){
            error_log("No closing ')'\n", mathOpName(elem_buffer->op));
            return false;
        }

        if (!short_scan)
            refillElemBuffer_(file, buffer, elem_buffer);
        return true;
    }
    error_log("Unexpected character %c found\n", c);
    return false;

}

static bool scanMathExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, MathElem* elem_buffer, int priority){
    assert_log(tree_place != nullptr);

    if (priority > MAX_MATH_OP_PRIORITY){
        return scanMathPExpr_(file, tree_place, buffer, elem_buffer);
    }
    BinTreeNode** tree_place_cur = tree_place;
    while (1){
        if (!scanMathExpr_(file, tree_place_cur, buffer, elem_buffer, priority+1))
            return false;
        if (elem_buffer->type == MATH_OP && isMathOpUnary(elem_buffer->op)){
            error_log("Unary operator %s used as binary\n", mathOpName(elem_buffer->op));
            return false;
        }
        binTreeUpdSize(*tree_place);

        if (elem_buffer->type == MATH_CNTRL)
            return true;
        if (elem_buffer->type != MATH_OP){
            error_log("Op required between two arguments\n");
            return false;
        }
        if (getMathOpPriority(elem_buffer->op) < priority)
            return true;



        BinTreeNode* old_node = *tree_place;
        BinTreeNode* new_node = binTreeNewNode(*elem_buffer);
        *tree_place = new_node;
        new_node->left = old_node;
        tree_place_cur = &(new_node->right);
        refillElemBuffer_(file, buffer, elem_buffer);
    }
    return false;
}

BinTreeNode* scanMathForm(FILE* file){
    char* buffer = (char*)calloc(MAX_FORM_WORD_LEN, sizeof(char));
    BinTreeNode* tree =  nullptr;
    MathElem elem_buffer = {};
    elem_buffer.type == MATH_PAIN;

    refillElemBuffer_(file, buffer, &elem_buffer);
    bool res = scanMathPExpr_(file, &tree, buffer, &elem_buffer, true);
    free(buffer);

    if (!res){
        binTreeDump(tree);
        binTreeDtor(tree);
        return nullptr;
    }
    return tree;
}
