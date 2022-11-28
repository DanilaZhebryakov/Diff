#include <iostream>
#include "lib/UStack.h"

#include "formule_utils.h"
#include "syntax_analyser.h"
#include "lib/bintree.h"

int main()
{
    //e^(ln(x)*z)
    BinTreeNode* tree   = scanMathForm(stdin);
    printMathForm(stdout, tree);
    binTreeDump(tree);

    BinTreeNode* diff = diffMathForm(tree, "");
    printf_log("Diff tree:\n");
    binTreeDump(diff);

    printf("\n");
    simplifyMathForm(&diff);
    printMathForm(stdout, diff);
    printf("\n");
    createMathFormVid(diff);



    binTreeDtor(tree);
    binTreeDtor(diff);

    return 0;
}
