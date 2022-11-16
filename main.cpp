#include <iostream>
#include "lib/UStack.h"
#include "formule_utils.h"
#include "lib/bintree.h"

BinTreeNode* scanMathForm(FILE* file);
void printMathForm(FILE* file, BinTreeNode* form);

int main()
{
    BinTreeNode* tree = scanMathForm(stdin);
    printMathForm(stdout, tree);

    return 0;
}
