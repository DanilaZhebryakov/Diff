#ifndef FORMULE_UTILS_H_INCLUDED
#define FORMULE_UTILS_H_INCLUDED

#include "math_elem.h"
#include "lib/bintree.h"

BinTreeNode* scanMathForm(FILE* file);

void printMathForm(FILE* file, BinTreeNode* form);

BinTreeNode* diffMathForm(BinTreeNode* form, const char* var);

#endif // FORMULE_UTILS_H_INCLUDED
