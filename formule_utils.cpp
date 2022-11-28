#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "math_elem.h"
#include "lib/bintree.h"

static BinTreeNode* newBadNode(){
    MathElem elem = {};
    elem.type = MATH_PAIN;
    return binTreeNewNode(elem, 0);
}

static BinTreeNode* newConstNode(double val){
    MathElem elem = {};
    elem.type = MATH_CONST;
    elem.val  = val;
    return binTreeNewNode(elem, 0);
}
static BinTreeNode* newVarNode(const char* name){
    MathElem elem = {};
    elem.type = MATH_VAR;
    elem.name = strdup(name);
    return binTreeNewNode(elem, 0);
}
static BinTreeNode* newOpNode(mathOpType_t op, BinTreeNode* a, BinTreeNode* b){
    MathElem elem = {};
    elem.type = MATH_OP;
    elem.op   = op;
    BinTreeNode* node = binTreeNewNode(elem, 0);
    if (a)
        binTreeAttach(a, node, BTREE_POS_LEFT );
    if (b)
        binTreeAttach(b, node, BTREE_POS_RIGHT);
    binTreeUpdSize(node);
    return node;
}

static bool mathElemEquals(BinTreeNode* node, double cmp_val){
    return node && (node->data.type == MATH_CONST &&
                    (node->data.val - cmp_val <= math_eps && (node->data.val - cmp_val >= -math_eps))
                    );
}

void printMathForm(FILE* file, BinTreeNode* form, int priority){
    if (!form)
        return;
    if (form->data.type == MATH_OP){
        int op_priority = getMathOpPriority(form->data.op);
        if(op_priority < priority){
            fprintf(file, "(");
        }
        if (isMathOpUnary(form->data.op)){
            printMathElem(file, form->data);
            fprintf(file, " ");
            printMathForm(file, form->right, op_priority);
        }
        else{
            printMathForm(file, form->left, op_priority);
            fprintf(file, " ");
            printMathElem(file, form->data);
            fprintf(file, " ");
            printMathForm(file, form->right, op_priority + 1);
        }
        if(op_priority < priority){
            fprintf(file, ")");
        }
        return;
    }

    if (form->data.type == MATH_FUNC){
        printMathElem(file, form->data);
        fprintf(file, "(");
        printMathForm(file, form->right, 0);
        fprintf(file, ")");
        return;
    }

    printMathElem(file, form->data);

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

void createMathFormVid(BinTreeNode* form){
    const int image_width = 700;
    const int image_frame_inc = 3;
    char buffer[1000] = {};

    FILE* form_file = fopen("form.txt", "w");
    fprintf(form_file,
"\\documentclass[10pt,a4paper]{article}\
\\usepackage[OT1]{fontenc}\n\
\\usepackage{amsmath}\n\
\\usepackage{amsfonts}\n\
\\usepackage{amssymb}\n\
\\usepackage{graphicx}\n\
\\usepackage{comment}\n\
\\usepackage{xcolor}\n\
\\pagecolor{black}\n\
\n\
\\begin{document}\n\
 \\color{white}\n\
 $"
    );
    printMathFormTex(form_file, form);
    fprintf(form_file, "$\n \\end{document}");
    fclose(form_file);

    system("pdflatex form.txt -quiet");
    const int max_i = image_width/image_frame_inc;
    for(int i = 0; i < max_i; i++){
        printf("\r");
        printf("Creating video frames (%d/%d):", i+1 , max_i);
        createNormalProgressBar(stdout, 20, 20*i/max_i);
        sprintf(buffer, "magick form.pdf -quiet  -crop %dx50+130+103 img/form.png",1 + i*image_frame_inc);
        system(buffer);
        sprintf(buffer, "magick img_backg.png img/form.png -gravity West -geometry +50+0 -compose blend -composite img/img_out%d.png" , i);
        system(buffer);

    }
    printf("\nProcessing video...");

    sprintf(buffer, "ffmpeg -framerate 10 -i \"img/img_out%%d.png\" -vframes %d -vcodec libx264 -crf 25  -pix_fmt yuv420p out_vid.mp4 -y", image_width/image_frame_inc);
    system(buffer);

}


BinTreeNode* replaceMathFormVar(BinTreeNode* form, const char* var, double val){
    if(!form){
        return nullptr;
    }
    if (form->data.type == MATH_CONST)
        return form;
    if (form->data.type == MATH_VAR){
        return (strcmp(var, form->data.name) == 0) ? newConstNode(val) : form;
    }
    if (form->data.type == MATH_OP){
        BinTreeNode* l = replaceMathFormVar(form->left, var, val);
        BinTreeNode* r = replaceMathFormVar(form->right, var, val);
        if(l == form->left && r == form->right){
            return form;
        }
        return newOpNode(form->data.op, l, r);
    }
    return nullptr;
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

    if (form->left->data.type == MATH_CONST){
        return newOpNode(MATH_O_MUL,
                        newOpNode(MATH_O_LN, nullptr, form->left),
                        newOpNode(MATH_O_MUL,
                             form,
                             diffMathForm_(form->right, var))
                    );
    }
    if (form->right->data.type == MATH_CONST){
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
    tmp_node->usedc++;
    BinTreeNode* ret = diffMathForm_(tmp_node, var);
    binTreeDtor(tmp_node);
    return ret;
}

static BinTreeNode* diffMathForm_(BinTreeNode* form, const char* var) {
    assert_log(form != nullptr);
    if (form->data.type == MATH_CONST){
        return newConstNode(0);
    }
    if (form->data.type == MATH_VAR){
        if (*var != '\0'){
            if (strcmp(form->data.name, var) == 0)
                return newConstNode(1);
            else
                return newConstNode(0);
        }
        else{
            return newOpNode(MATH_O_d, nullptr, form);
        }
    }
    if (form->data.type == MATH_OP){

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
        case MATH_O_o:
            return newConstNode(0);
        default:
            return newBadNode();
        }

    }
    return newBadNode();
}

BinTreeNode* diffMathForm(BinTreeNode* form, const char* var) {
    assert_log(var);
    assert_log(form);

    BinTreeNode* ret = diffMathForm_(form, var);
    if (!ret){
        return nullptr;
    }
    ret->usedc++;
    return ret;
}

BinTreeNode* diffMathForm_n(BinTreeNode* form, const char* var, int n) {
    if (n == 0){
        form->usedc++;
        return form;
    }
    form = diffMathForm(form, var);
    for (int i = 1; i < n; i++){
        BinTreeNode* t = diffMathForm(form, var);
        binTreeNodeDtor(form);
        form = t;
        if (!form)
            return nullptr;
    }
    return form;
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

    if (form->data.type != MATH_OP)
        return form;

    simplifyMathForm(&(form->right));
    if (!isMathOpUnary(form->data.op)){
        simplifyMathForm(&(form->left));
    }
    if (!form->right)
        return newBadNode();
    if (!(form->left || isMathOpUnary(form->data.op)))
        return newBadNode();


    if (form->left && form->right->data.type == MATH_CONST && form->left->data.type == MATH_CONST){
        double l = form->left ->data.val;
        double r = form->right->data.val;
        double ans = calcMathOp(form->data.op, l, r);
        if(!(ans == ans)){
            return form;
        }
        return newConstNode(ans);
    }
    if (!form->left && form->right->data.type == MATH_CONST){
        double r = form->right->data.val;
        double ans = calcMathOp(form->data.op, NAN, r);
        if(!(ans == ans)){
            return form;
        }
        return newConstNode(ans);
    }

    if (mathElemEquals(form->right, 0)){
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
    if (mathElemEquals(form->left, 0)){
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
    if (mathElemEquals(form->right, 1)){
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
    if (mathElemEquals(form->left, 0)){
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

static unsigned long long factorial(int n){
    unsigned long long r = 1;
    for (int i = 1; i <= n; i++){
        r = r * i;
    }
    return r;
}

BinTreeNode* taylorMathForm(BinTreeNode* form, const char* var, double point, int o_st){
    BinTreeNode* d_n = diffMathForm(form, var);
    simplifyMathForm(&d_n);
    d_n->usedc--;
    BinTreeNode* var_node = newVarNode(var);
    BinTreeNode* x_sub_point = newOpNode(MATH_O_SUB, var_node, newConstNode(point));
    BinTreeNode* res = replaceMathFormVar(form, var, point);

    for (int i = 1; i <= o_st; i++){
        BinTreeNode* s_mem = newOpNode(MATH_O_MUL,
                                    newOpNode(MATH_O_DIV,
                                        newOpNode(MATH_O_POW,
                                            x_sub_point,
                                            newConstNode(i)
                                        ),
                                        newConstNode(factorial(i))
                                    ),
                                    replaceMathFormVar(d_n, var, point)
                                );
        res = newOpNode(MATH_O_ADD, res, s_mem);
        d_n = diffMathForm(d_n, var);
        simplifyMathForm(&d_n);
        d_n->usedc--;
    }
    res =   newOpNode(MATH_O_ADD, res,
                newOpNode(MATH_O_o, nullptr,
                    newOpNode(MATH_O_POW,
                        x_sub_point,
                        newConstNode(o_st)
                    )
                )
            );
    res->usedc++;
    return res;
}



