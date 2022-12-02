#ifndef FORMULE_UTILS_H_INCLUDED
#define FORMULE_UTILS_H_INCLUDED

#include "math_elem.h"
#include "lib/bintree.h"

void printMathForm(FILE* file, BinTreeNode* form, int priority = 0);

void printMathFormTex(FILE* file, BinTreeNode* form);

BinTreeNode* requestAllVars(BinTreeNode* form);

void createMathFormVid(BinTreeNode* form);

BinTreeNode* replaceMathFormVar(BinTreeNode* form, const char* var, double val);

BinTreeNode* diffMathForm(BinTreeNode* form, const char* var);

BinTreeNode* diffMathForm_n(BinTreeNode* form, const char* var, int n);

void simplifyMathForm(BinTreeNode** form);

BinTreeNode* taylorMathForm(BinTreeNode* form, const char* var, double point, int o_st);


#endif // FORMULE_UTILS_H_INCLUDED
