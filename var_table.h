#ifndef VAR_TABLE_H_INCLUDED
#define VAR_TABLE_H_INCLUDED

#include "lib/UStack.h"
typedef UStack VarTable;

struct VarEntry{
    double value;
    char*  name;
    int depth;
};

bool varTablePut(VarTable* stk, VarEntry var);

VarEntry* varTableGet(VarTable* stk, char* s_name);

VarEntry* varTableGetLast(VarTable* stk);

#endif // VAR_TABLE_H_INCLUDED
