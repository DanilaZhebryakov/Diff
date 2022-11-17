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

    return 0;
}
