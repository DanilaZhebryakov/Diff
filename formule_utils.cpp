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
            if(!(old_node || isMathOpUnary(data.op))){
                error_log("no arg");
                return false;
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
        if(isMathOpUnary(form->data.op)){
            printMathElem(file, form->data);
            fprintf(file, "(");
            printMathForm(file, form->right);
            fprintf(file, ")");
        }
        else{
            fprintf(file, "(");
            printMathForm(file, form->left);
            fprintf(file, " ");
            printMathElem(file, form->data);
            fprintf(file, " ");
            printMathForm(file, form->right);
            fprintf(file, ")");
        }
    }
    else{
        printMathElem(file, form->data);
    }

}

void printMathFormTex(FILE* file, BinTreeNode* form){
    if (!form)
        return;

    if (form->data.type == MATH_OP){
        switch(form->data.op){
            case MATH_O_DIV:
                fprintf(file, "\\frac{");
                printMathFormTex(file, form->left);
                fprintf(file, "}{");
                printMathFormTex(file, form->right);
                fprintf(file, "}");
                break;
            case MATH_O_SQRT:
                fprintf(file, "\\sqrt{");
                printMathFormTex(file, form->right);
                fprintf(file, "}");
                break;
            case MATH_O_POW:
                fprintf(file, "{(");
                printMathFormTex(file, form->left);
                fprintf(file, ")}^{");
                printMathFormTex(file, form->right);
                fprintf(file, "}");
                break;
            case MATH_O_LOG:
                fprintf(file, "log_{");
                printMathFormTex(file, form->left);
                fprintf(file, "}{(");
                printMathFormTex(file, form->right);
                fprintf(file, ")}");
                break;
            default:
                if(isMathOpUnary(form->data.op)){
                    printMathElem(file, form->data);
                    fprintf(file, "(");
                    printMathFormTex(file, form->right);
                    fprintf(file, ")");
                }
                else{
                    fprintf(file, "(");
                    printMathFormTex(file, form->left);
                    fprintf(file, " ");
                    printMathElem(file, form->data);
                    fprintf(file, " ");
                    printMathFormTex(file, form->right);
                    fprintf(file, ")");
                }
            break;
        }
    }
    else{
        printMathElem(file, form->data);
    }
}

BinTreeNode* newBadNode(){
    MathElem elem = {};
    elem.type = MATH_PAIN;
    return binTreeNewNode(elem, 0);
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
    if(a)
        binTreeAttach(a, node, BTREE_POS_LEFT );
    if(b)
        binTreeAttach(b, node, BTREE_POS_RIGHT);
    binTreeUpdSize(node);
    return node;
}


//diff block

static BinTreeNode* diffMathForm_(BinTreeNode* form, const char* var);

static BinTreeNode* diffLog_(BinTreeNode* form, const char* var){
    assert_log(form != nullptr);
    assert_log(form->left  != nullptr);
    assert_log(form->right != nullptr);

    BinTreeNode* tmp_node = newOpNode(MATH_O_DIV,
                                        newOpNode(MATH_O_LN, nullptr, form->left ),
                                        newOpNode(MATH_O_LN, nullptr, form->right)
                                    );
    BinTreeNode* ret = diffMathForm_(tmp_node, var);
    binTreeNodeDtor(tmp_node);
    return ret;
}

static BinTreeNode* diffPow_(BinTreeNode* form, const char* var){
    assert_log(form != nullptr);
    assert_log(form->left  != nullptr);
    assert_log(form->right != nullptr);

    if(form->left->data.type == MATH_CONST){
        return newOpNode(MATH_O_MUL,
                        newOpNode(MATH_O_LN, nullptr, form->left),
                        newOpNode(MATH_O_MUL,
                             form,
                             diffMathForm_(form->right, var))
                    );
    }
    if(form->right->data.type == MATH_CONST){
        return newOpNode(MATH_O_MUL,
                    newOpNode(MATH_O_MUL,
                        form->right,
                        newOpNode(MATH_O_POW,
                            form->left,
                            newConstNode(form->right->data.val-1)
                        )
                    ),
                    diffMathForm_(form->left, var)
                );
    }

    BinTreeNode* tmp_node = newOpNode(MATH_O_EXP, nullptr,
                                    newOpNode(MATH_O_MUL,
                                        newOpNode(MATH_O_LN, nullptr, form->left),
                                        form->right
                                    )
                            );
    BinTreeNode* ret = diffMathForm_(tmp_node, var);
    binTreeNodeDtor(tmp_node);
    return ret;
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
        case MATH_O_EXP:
            return newOpNode(MATH_O_MUL,
                             form,
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_LN:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_DIV, newConstNode(1), form->right),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_SIN:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_COS, nullptr, form->right),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_COS:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_MUL,
                                newOpNode(MATH_O_SIN, nullptr, form->right),
                                newConstNode(-1)),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_TG:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_DIV,
                                newConstNode(1),
                                newOpNode(MATH_O_POW,
                                    newOpNode(MATH_O_COS, nullptr, form->right),
                                    newConstNode(2)
                                )
                             ),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_SH:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_CH, nullptr, form->right),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_CH:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_SH, nullptr, form->right),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_TH:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_DIV,
                                newConstNode(1),
                                newOpNode(MATH_O_POW,
                                    newOpNode(MATH_O_CH, nullptr, form->right),
                                    newConstNode(2)
                                )
                             ),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_ASIN:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_DIV,
                                newConstNode(1),
                                newOpNode(MATH_O_SQRT, nullptr,
                                    newOpNode(MATH_O_SUB,
                                        newConstNode(1),
                                        newOpNode(MATH_O_POW, form->right, newConstNode(2))
                                    )
                                )
                             ),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_ACOS:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_DIV,
                                newConstNode(-1),
                                newOpNode(MATH_O_SQRT, nullptr,
                                    newOpNode(MATH_O_SUB,
                                        newConstNode(1),
                                        newOpNode(MATH_O_POW, form->right, newConstNode(2))
                                    )
                                )
                             ),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_ATG:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_DIV,
                                newConstNode(1),
                                newOpNode(MATH_O_ADD,
                                    newConstNode(1),
                                    newOpNode(MATH_O_POW, form->right, newConstNode(2))
                                )
                             ),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_SQRT:
            return newOpNode(MATH_O_MUL,
                             newOpNode(MATH_O_DIV, newConstNode(0.5), form),
                             diffMathForm_(form->right, var)
                             );
        case MATH_O_POW:
            return diffPow_(form, var);
        case MATH_O_LOG:
            return diffLog_(form, var);
        default:
            return newBadNode();
        }

    }
    return newBadNode();
}

BinTreeNode* diffMathForm(BinTreeNode* form, const char* var) {
    BinTreeNode* ret = diffMathForm_(form, var);
    if(!ret){
        return nullptr;
    }
    ret->usedc++;
    return ret;
}

bool mathElemEquals(BinTreeNode* node, double cmp_val){
    return node && (node->data.type == MATH_CONST &&
                    (node->data.val - cmp_val <= math_eps && (node->data.val - cmp_val >= -math_eps))
                    );
}


// simplify block

static BinTreeNode* simplifyMathForm_(BinTreeNode* form);

void simplifyMathForm(BinTreeNode** form){
    assert_log(form != nullptr);
    BinTreeNode* newnode = simplifyMathForm_(*form);
    newnode->usedc++;
    binTreeUpdSize(newnode);
    binTreeDtor(*form);
    *form = newnode;

}

static BinTreeNode* simplifyMathForm_(BinTreeNode* form){
    assert_log(form != nullptr);

    if(form->data.type != MATH_OP)
        return form;

    simplifyMathForm(&(form->right));
    if(!isMathOpUnary(form->data.op)){
        simplifyMathForm(&(form->left));
    }
    if(!form->right)
        return newBadNode();
    if(!(form->left || isMathOpUnary(form->data.op)))
        return newBadNode();


    if(form->right->data.type == MATH_CONST && (!form->left || form->left->data.type == MATH_CONST)){
        double l = form->left ? form->left ->data.val : (0/0);
        double r =              form->right->data.val;
        return newConstNode(calcMathOp(form->data.op, l, r));
    }

    if(mathElemEquals(form->right, 0)){
        switch(form->data.op){
            case MATH_O_ADD:
                return form->left;
            case MATH_O_SUB:
                return form->left;
            case MATH_O_MUL:
                return newConstNode(0);
            case MATH_O_POW:
                return newConstNode(1);
            default:
                break;
        }
    }
    if(mathElemEquals(form->left, 0)){
        switch(form->data.op){
            case MATH_O_ADD:
                return form->right;
            case MATH_O_DIV:
                return newConstNode(0);
            case MATH_O_MUL:
                return newConstNode(0);
            case MATH_O_POW:
                return newConstNode(1);
            default:
                break;
        }
    }
    if(mathElemEquals(form->right, 1)){
        switch(form->data.op){
            case MATH_O_MUL:
                return form->left;
            case MATH_O_DIV:
                return form->left;
            case MATH_O_POW:
                return form->left;
            default:
                break;
        }
    }
    if(mathElemEquals(form->left, 0)){
        switch(form->data.op){
            case MATH_O_MUL:
                return form->right;
            case MATH_O_POW:
                return newConstNode(1);
            case MATH_O_LOG:
                return newConstNode(0);
            default:
                break;
        }
    }
    return form;
}
