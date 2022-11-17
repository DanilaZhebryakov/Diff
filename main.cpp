#include <iostream>
#include "lib/UStack.h"
#include "formule_utils.h"
#include "lib/bintree.h"

int main()
{
    BinTreeNode* tree = scanMathForm(stdin);
    printMathForm(stdout, tree);

    return 0;
}
