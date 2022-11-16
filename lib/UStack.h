#ifndef USTACK_H_INCLUDED
#define USTACK_H_INCLUDED

#include "logging.h"
#include "debug_utils.h"

struct UStack{
    #ifndef STACK_NO_CANARY
        canary_t leftcan;
    #endif

    void*  data;
    size_t elem_size;
    size_t size;
    size_t capacity;

    #ifndef STACK_NO_CANARY
        canary_t rightcan;
    #endif
};

#define ustackCtor(_stk, _elsize) { \
    ustackCtor_(_stk, _elsize);     \
}

bool ustackCtor_(UStack* stk, size_t elsize);

varError_t ustackError(const UStack* stk);

void ustackDump(const UStack* stk);

varError_t ustackDtor(UStack* stk);

varError_t ustackResize(UStack* stk, size_t new_capacity);

varError_t ustackPush(UStack* stk, void* elem_ptr);

varError_t ustackTop(UStack* stk, void* elem_ptr);

varError_t ustackGet(UStack* stk, size_t addr, void* elem_ptr);

varError_t ustackPop(UStack* stk, void* elem_ptr);

#endif // USTACK_H_INCLUDED
