#include <stdio.h>
#include <ctype.h>

#include "math_elem.h"
#include "lib/bintree.h"

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
