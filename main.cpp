#include <iostream>
#include "lib/UStack.h"

#include "formule_utils.h"
#include "syntax_analyser.h"
#include "lib/bintree.h"

int main()
{
    //e^(ln(x)*z)
    BinTreeNode* tree   = scanMathForm(stdin);
    simplifyMathForm(&tree);
    printMathForm(stdout, tree);
    binTreeDump(tree);
    printf("\n");

    printf("Select mode: d = diff, D = diff n times, t = taylor, other for do nothing\n"
           "Modifiers: + to enter values, # to create video\n");
    char inp_str[100] = {};
    scanf(" %s", inp_str);
    getchar(); //skip \n
    int t = 0;
    double p = 0;
    char inp_var[1000] = {};
    BinTreeNode* res = nullptr;
    switch(*inp_str){
    case 'd':
        printf("Enter var for diff (blank for full diff)\n");
        gets(inp_var);
        res = diffMathForm(tree, inp_var);
        break;
    case 'D':
        printf("Enter var for diff (blank for full diff)\n");
        gets(inp_var);
        printf("Enter diff number\n");
        scanf(" %d", &t);
        res = diffMathForm_n(tree, inp_var, t);
        break;
    case 't':
        printf("Enter var for taylor\n");
        gets(inp_var);
        printf("Enter length\n");
        scanf(" %d", &t);
        printf("Enter point\n");
        scanf(" %lf", &p);
        res = taylorMathForm(tree, inp_var , p, t);
        break;
    default:
        res = tree;
        tree->usedc++;
        break;
    }
    simplifyMathForm(&res);
    binTreeDump(res);

    if(strchr(inp_str, '#')){
        createMathFormVid(res);
    }

    if(strchr(inp_str, '+')){
        BinTreeNode* t_node = requestAllVars(res);
        binTreeDtor(res);
        res = t_node;
        simplifyMathForm(&res);
    }

    printMathForm(stdout, res);
    printf("\n");


    binTreeDtor(tree);
    binTreeDtor(res);

    return 0;
}
