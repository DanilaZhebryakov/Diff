#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "asserts.h"
#include "logging.h"
#include "debug_utils.h"

#include "UStack.h"

#define USTACK_BADMEM 0xEF
#define DESTRUCT_PTR  ((void*)666)
#define USTACK_MIN_SIZE 10

#ifndef USTACK_NO_CANARY
    static const size_t USTACK_DATA_BEGIN_OFFSET  =   sizeof(canary_t);
    static const size_t USTACK_DATA_SIZE_OFFSET   = 2*sizeof(canary_t);
#else
    static const size_t USTACK_DATA_BEGIN_OFFSET  = 0;
    static const size_t USTACK_DATA_SIZE_OFFSET   = 0;
#endif

#ifndef USTACK_NO_PROTECT
    #define ustackCheckRet(__stk, ...)  \
        if(ustackError(__stk)){             \
            Error_log("%s", "Stack error");\
            ustackDump(__stk);              \
            return __VA_ARGS__;            \
        }
#else
    #define uStackCheckRet(__stk, ...) ;
#endif

#ifndef USTACK_NO_PROTECT
    #define ustackCheckRetPtr(__stk, __errptr, ...)  \
        if(ustackError(__stk)){                \
            Error_log("%s", "UStack error");   \
            ustackDump(__stk);                 \
            if(__errptr)                      \
                *__errptr = ustackError(__stk);\
            return __VA_ARGS__;               \
        }
#else
    #define ustackCheckRetPtr(__stk, __errptr, ...)  ;
#endif

inline static void* ustackDataMemBegin(const UStack* stk){
    assert_log(stk != nullptr);
    return ((char*)stk->data)-USTACK_DATA_BEGIN_OFFSET;
}
inline static size_t ustackDataMemSize(const UStack* stk){
    assert_log(stk != nullptr);
    return (stk->capacity*stk->elem_size) + USTACK_DATA_SIZE_OFFSET ;
}

#define ustackCtor(_stk, _elsize) { \
    ustackCtor_(_stk, _elsize);     \
}


bool ustackCtor_(UStack* stk, size_t elsize){
    #ifndef USTACK_NO_PROTECT
    if (IsBadWritePtr(stk, sizeof(stk))){
        return false;
    }
    #endif
    stk->data = nullptr;
    stk->size = 0;
    stk->capacity = 0;
    stk->elem_size = elsize;

    #ifndef USTACK_NO_CANARY
        stk->leftcan  = CANARY_L;
        stk->rightcan = CANARY_R;
    #endif
    return true;
}

varError_t ustackError(const UStack* stk){
    if (stk == nullptr)
        return VAR_NULL;

    if (IsBadReadPtr(stk, sizeof(stk)))
        return VAR_BAD;

    if (stk->size == SIZE_MAX || stk->capacity == SIZE_MAX || stk->data == DESTRUCT_PTR)
        return VAR_DEAD;


    unsigned int err = 0;

    if (stk->capacity != 0){
        if (stk->data == nullptr)
            err |= VAR_DATA_NULL;
        if (IsBadWritePtr(ustackDataMemBegin(stk), ustackDataMemSize(stk)))
            err |= VAR_DATA_BAD;
    }
    if (stk->size > stk->capacity)
        err |= VAR_BADSTATE;

    #ifndef USTACK_NO_CANARY
        if (stk->leftcan != CANARY_L)
            err |= VAR_CANARY_L_BAD;
        if (stk->rightcan != CANARY_R)
            err |= VAR_CANARY_R_BAD;
    #endif



    if ((err & (VAR_DATA_BAD | VAR_HASH_BAD)) || stk->data == nullptr){
        return (varError_t)err;
    }

    #ifndef USTACK_NO_CANARY
        if (!checkLCanary(stk->data))
            err |= VAR_DATA_CANARY_L_BAD;
        if (!checkRCanary(stk->data, stk->capacity * stk->elem_size))
            err |= VAR_DATA_CANARY_R_BAD;
    #endif

    return (varError_t)err;
}

static varError_t ustackError_dbg(UStack* stk){
    #ifndef USTACK_NO_PROTECT
        return ustackError(stk);
    #else
        return VAR_NOERROR;
    #endif
}


void ustackDump(const UStack* stk){

    info_log("UStack dump:\n      stack at %p \n", stk);

    varError_t err = ustackError(stk);
    if (err & VAR_NULL){
        printf_log("      (BAD)  Stack poiner is null\n");
        return;
    }
    if (err & VAR_BAD){
        printf_log("      (BAD)  Stack poiner is invalid\n");
        return;
    }

    printf_log("      %ld/%ld elements (%d bytes each)\n", stk->size, stk->capacity, stk->elem_size);
    printf_log("      Data: %p\n", stk->data);

    if (err & VAR_DEAD){
        printf_log("      (BAD)  Stack was already destructed\n\n");
        return;
    }

    #ifndef USTACK_NO_CANARY
        if (err & VAR_CANARY_L_BAD){
            printf_log("      (BAD)  Struct L canary BAD! Value: %p\n", stk->leftcan);
        }
        if (err & VAR_CANARY_R_BAD){
            printf_log("      (BAD)  Struct R canary BAD! Value: %p\n", stk->rightcan);
        }
    #endif

    if (stk->data == nullptr){
        printf_log("      (bad?) Stack data poiner is null\n\n");
        return;
    }

    if (err & VAR_DATA_BAD){
        printf_log("      (BAD)  Stack data poiner is invalid\n");
        dumpData(ustackDataMemBegin(stk), ustackDataMemSize(stk));
        return;
    }
    if (err & VAR_BADSTATE){
        printf_log("      (BAD)  Stack size is larger than capacity\n");
    }
    #ifndef USTACK_NO_CANARY
        if (!checkLCanary(stk->data)){
            printf_log("      (BAD)  Data L canary BAD! Value: %p\n", ((canary_t*)stk->data)[-1]);
        }
        if (!checkRCanary(stk->data, stk->capacity * stk->elem_size)){
            printf_log("      (BAD)  Data R canary BAD! Value: %p\n",*((canary_t*)stk->data + stk->capacity));
        }
    #endif
    printf_log("\n");

    for (size_t i = 0; i < stk->capacity; i++){
        printf_log("    ");

        printf_log("%c", (i < stk->size           ) ? '*':' ');

        printf_log("[%ld] ", i);
        for (size_t j = 0; j < stk->elem_size; j++){
            printf_log("%02X", ((char*)stk->data)[i*(stk->elem_size) + j]);
        }
        printf_log("\n");
        //printf_log(" %s\n", (stk->data[i] == BAD_ELEM) ? "(POISON)":"");
    }
    printf_log("\n");

}

varError_t ustackDtor(UStack* stk){
    ustackCheckRet(stk, ustackError_dbg(stk));

    #ifndef USTACK_NO_PROTECT
        memset(stk->data, USTACK_BADMEM, stk->capacity * stk->elem_size);
    #endif
    if (stk->data != nullptr)
        free(ustackDataMemBegin(stk));

    #ifndef USTACK_NO_PROTECT
        stk->data = DESTRUCT_PTR;
        stk->size = -1;
        stk->capacity = -1;
    #endif USTACK_NO_PROTECT
    return VAR_NOERROR;
}



static varError_t ustackResize_(UStack* stk, size_t new_capacity){

    int err = ustackError_dbg(stk);
    err &= ~(VAR_HASH_BAD | VAR_DATA_HASH_BAD);
    if (err)
        return (varError_t)err;


    if (new_capacity < stk->size){
        return VAR_BADOP;
    }

    errno = 0;
    void* new_mem = nullptr;
    if (stk->data != nullptr){
        new_mem = ((char*)realloc(ustackDataMemBegin(stk), new_capacity*(stk->elem_size) + USTACK_DATA_SIZE_OFFSET)
                            + USTACK_DATA_BEGIN_OFFSET);
    }
    else{
        new_mem = ((char*)calloc(                          new_capacity*(stk->elem_size) +  USTACK_DATA_SIZE_OFFSET, 1)
                            + USTACK_DATA_BEGIN_OFFSET);
    }
    if (new_mem == nullptr){
        perror_log("error while reallocating memory for stack");
        return VAR_INTERR;
    }
    stk->data = new_mem;

    #ifndef USTACK_NO_CANARY
        *((canary_t*)((char*)stk->data + (new_capacity * stk->elem_size))) = CANARY_R;
        *((canary_t*)(       stk->data)-1)                                 = CANARY_L;
    #endif

    #ifndef USTACK_NO_PROTECT
        memset(stk->data + (stk->capacity * stk->elem_size), USTACK_BADMEM, (new_capacity - stk->capacity) * stk->elem_size);
    #endif
    stk->capacity = new_capacity;
    return VAR_NOERROR;
}

varError_t ustackResize(UStack* stk, size_t new_capacity){
    varError_t err = ustackResize_(stk, new_capacity);
    return err;
}

varError_t ustackPush(UStack* stk, void* elem_ptr){
    assert_ret(elem_ptr != nullptr, VAR_BADOP);
    ustackCheckRet(stk, ustackError(stk));

    if (stk->size == stk->capacity){
        varError_t err = ustackResize_(stk, (stk->capacity == 0)? USTACK_MIN_SIZE : stk->capacity*2);
        if (err != VAR_NOERROR)
            return err;
    }

    memcpy(((char*)stk->data) + ((stk->size++) * stk->elem_size), elem_ptr, stk->elem_size);

    return ustackError_dbg(stk);
}

varError_t ustackTop(UStack* stk, void* elem_ptr){
    assert_ret(elem_ptr != nullptr, VAR_BADOP);
    ustackCheckRet(stk, ustackError(stk));

    if (stk->size == 0){
        return VAR_BADOP;
    }
    memcpy(elem_ptr, ((char*)stk->data) + ((stk->size-1) * stk->elem_size), stk->elem_size);
    return VAR_NOERROR;
}

varError_t ustackGet(UStack* stk, size_t addr, void* elem_ptr){
    assert_ret(elem_ptr != nullptr, VAR_BADOP);
    ustackCheckRet(stk, ustackError(stk));

    if (stk->size <= addr){
        return VAR_BADOP;
    }
    memcpy(elem_ptr, ((char*)stk->data) + ((stk->size-1-addr) * stk->elem_size), stk->elem_size);
    return VAR_NOERROR;
}

varError_t ustackPop(UStack* stk, void* elem_ptr){
    varError_t err = VAR_ERRUNK;
    err = ustackTop(stk, elem_ptr);
    if (err != VAR_NOERROR){
        return err;
    }
    stk->size--;
    #ifndef USTACK_NO_PROTECT
        memset(stk->data + (stk->size * stk->elem_size), USTACK_BADMEM, stk->elem_size);
    #endif

    if (stk->size * 2 < stk->capacity && stk->capacity > 2*USTACK_MIN_SIZE){
        err = ustackResize_(stk, (stk->capacity == 0)? USTACK_MIN_SIZE : stk->size*2);
        if (err != VAR_NOERROR){
            return err;
        }
    }
    return VAR_NOERROR;
}
