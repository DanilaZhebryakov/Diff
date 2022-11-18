#include <iostream>
#include "lib/UStack.h"
#include "formule_utils.h"
#include "lib/bintree.h"

int main()
{
    BinTreeNode* tree = scanMathForm(stdin);
    printMathForm(stdout, tree);
    printf("\n");
    BinTreeNode* diff = diffMathForm(tree, "x");

    printMathForm(stdout, diff);
    printf("\n");

    simplifyMathForm(&diff);

    printMathForm(stdout, diff);
    printf("\n");

    binTreeDump(diff);
    binTreeDtor(tree);
    binTreeDtor(diff);
    return 0;
}
