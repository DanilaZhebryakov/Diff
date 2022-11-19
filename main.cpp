#include <iostream>
#include "lib/UStack.h"
#include "formule_utils.h"
#include "lib/bintree.h"

int main()
{
    BinTreeNode* tree   = scanMathForm(stdin);
    printMathForm(stdout, tree);
    BinTreeNode* taylor = taylorMathForm(tree, "x", 0, 7);
    printf("\n");
    simplifyMathForm(&taylor);
    printMathForm(stdout, taylor);
    printf("\n");

    binTreeDump(taylor);
    binTreeDtor(tree);
    binTreeDtor(taylor);
    return 0;
}
