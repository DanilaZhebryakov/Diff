#include <stdlib.h>
#include "var_table.h"

bool varTablePut(VarTable* stk, VarEntry var){
    return ustackPush(stk, &var) == VAR_NOERROR;
}
VarEntry* varTableGet(VarTable* stk, char* s_name){
    for (VarEntry* i = (VarEntry*)stk->data; i < ((VarEntry*)stk->data) + stk->size; i++){
        if(strcmp(i->name, s_name) == 0){
            return i;
        }
    }
    return nullptr;
}

VarEntry* varTableGetLast(VarTable* stk){
    return ((VarEntry*)stk->data) + stk->size - 1;
}
