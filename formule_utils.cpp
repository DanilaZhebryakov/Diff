#include <stdio.h>
#include <ctype.h>

#include "math_elem.h"
#include "lib/bintree.h"

static bool scanMathForm_(FILE* file, BinTreeNode** tree_place, char* buffer, bool full_scan = false){
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
        MathElem data = scanMathElem(file, c, buffer);
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

BinTreeNode* newConstNode(double val){
    MathElem elem = {};
    elem.type = MATH_CONST;
    elem.val  = val;
    return binTreeNewNode(elem, 0);
}
BinTreeNode* newVarNode(const char* name){
    MathElem elem = {};
    elem.type = MATH_VAR;
    elem.name = strdup(name);
    return binTreeNewNode(elem, 0);
}
BinTreeNode* newOpNode(mathOpType_t op, BinTreeNode* a, BinTreeNode* b){
    MathElem elem = {};
    elem.type = MATH_OP;
    elem.op   = op;
    BinTreeNode* node = binTreeNewNode(elem, 0);
    binTreeAttach(a, node, BTREE_POS_LEFT );
    binTreeAttach(b, node, BTREE_POS_RIGHT);
    binTreeUpdSize(node);
    return node;
}


static BinTreeNode* diffMathForm_(BinTreeNode* form, const char* var) {
    assert_log(form != nullptr);
    if(form->data.type == MATH_CONST){
        return newConstNode(0);
    }
    if(form->data.type == MATH_VAR){
        if(strcmp(form->data.name, var) == 0)
            return newConstNode(1);
        else
            return newConstNode(0);
    }
    if(form->data.type == MATH_OP){

        switch(form->data.op){
        case MATH_O_ADD:
            return newOpNode(MATH_O_ADD,
                             diffMathForm_(form->left , var),
                             diffMathForm_(form->right, var));
        case MATH_O_SUB:
            return newOpNode(MATH_O_SUB,
                             diffMathForm_(form->left , var),
                             diffMathForm_(form->right, var));
        case MATH_O_MUL:
            return newOpNode(MATH_O_ADD,
                             newOpNode(MATH_O_MUL,
                                        diffMathForm_(form->left, var),
                                        form->right),
                             newOpNode(MATH_O_MUL,
                                        form->left,
                                        diffMathForm_(form->right, var))
                             );
        case MATH_O_DIV:
            return newOpNode(MATH_O_DIV,
                        newOpNode(MATH_O_SUB,
                             newOpNode(MATH_O_MUL,
                                        diffMathForm_(form->left, var),
                                        form->right),
                             newOpNode(MATH_O_MUL,
                                        form->left,
                                        diffMathForm_(form->right, var))
                             ),
                        newOpNode(MATH_O_MUL, form->right, form->right));
        default:
            return nullptr;
        }

    }
    return nullptr;
}

BinTreeNode* diffMathForm(BinTreeNode* form, const char* var) {
    BinTreeNode* ret = diffMathForm_(form, var);
    if(!ret){
        return nullptr;
    }
    ret->usedc++;
    return ret;
}
