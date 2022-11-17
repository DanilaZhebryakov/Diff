#include <stdio.h>
#include <ctype.h>

#include "lib/logging.h"
#include "lib/debug_utils.h"
#include "lib/System_utils.h"
#include "bintree.h"

#define DESTRUCT_PTR ((void*)666)

#ifndef BINTREE_NO_PROTECT
    #define binTreeCheckRet(_node, ...)  \
        if(binTreeError_dbg(_node)){             \
            Error_log("%s", "Tree error");\
            binTreeDump(_node);              \
            return __VA_ARGS__;            \
        }
#else
    #define binTreeCheckRet(_node, ...)  ;
#endif

#ifndef BINTREE_NO_PROTECT
    #define binTreeCheckRetPtr(_node, __errptr, ...)  \
        if(binTreeError_dbg(_node)){                \
            Error_log("%s", "Tree error");   \
            binTreeDump(_node);                 \
            if(__errptr)                      \
                *__errptr = binTreeError_dbg(_node);\
            return __VA_ARGS__;               \
        }
#else
    #define binTreeCheckRet(_node, ...)  ;
#endif

#define   setNodeUsedFlag(_node) node->tmp = 1
#define clearNodeUsedFlag(_node) node->tmp = 0

static BINTREE_ELEM_T readElem(FILE* file){
    char c = fgetc(file);
    ungetc(c, file);

    char* buffer = (char*)calloc(MAX_FORM_WORD_LEN, sizeof(char));
    MathData elem = scanMathElem(file, c, buffer);
    free(buffer);

    return elem;
}

static void dumpElem(FILE* file, const BINTREE_ELEM_T* elem){
    printMathElem(file, *elem);
}

static void freeElem(BINTREE_ELEM_T elem){
    return;
}

static int elemCompare(BINTREE_ELEM_T a, BINTREE_ELEM_T b){
    Error_log("%s", "Comparator not implemented");
    return false;
}

void binTreeNodeCtor(BinTreeNode* node, BINTREE_ELEM_T elem, int use){
    node->data  = elem;
    node->left  = nullptr;
    node->right = nullptr;
    #ifdef BINTREE_BACK_LINK
    node->up    = nullptr;
    #endif

    #ifndef BINTREE_NO_PROTECT
    node->tmp = 0;
    #endif
    #ifdef BINTREE_STORE_SIZE
    node->size = 1;
    #endif
    node->usedc = use;
}

BinTreeNode* binTreeNewNode(BINTREE_ELEM_T elem, int use){
    BinTreeNode* ret = (BinTreeNode*)malloc(sizeof(BinTreeNode));
    binTreeNodeCtor(ret, elem, use);
    return ret;
}

void binTreeNodeDtor(BinTreeNode* node){
    freeElem(node->data);
    #ifndef BINTREE_NOPROTECT
        node->data  = BINTREE_BADELEM;
        node->left  = (BinTreeNode*)DESTRUCT_PTR;
        node->right = (BinTreeNode*)DESTRUCT_PTR;
        #ifdef BINTREE_BACK_LINK
        node->up    = (BinTreeNode*)DESTRUCT_PTR;
        #endif
        node->usedc = -1;
    #endif
    free(node);
}

void binTreeDtor(BinTreeNode* node){
    if(!node)
        return;

    node->usedc--;
    if(node->usedc <= 0){
        if(node->left){
            binTreeDtor(node->left);
        }
        if(node->right){
            binTreeDtor(node->right);
        }
        binTreeNodeDtor(node);
    }
}

static binTreeError_t passErrFromNode(binTreeError_t err){
    int err_i = err;
    if(err & BTREE_NULL){
        err_i ^= BTREE_NULL;
    }
    if(err & BTREE_BAD){
        err_i ^= BTREE_BAD;
        err_i |= BTREE_BADNODE;
    }
    if(err & BTREE_DEAD){
        err_i ^= BTREE_DEAD;
        err_i |= BTREE_DEADNODE;
    }
    return (binTreeError_t)err_i;
}

binTreeError_t binTreeError_base(BinTreeNode* node, bool check_loop){
    if (node == nullptr)
        return BTREE_NULL;

    if (!isPtrWritable(node, sizeof(node)))
        return BTREE_BAD;

    if (node->left == DESTRUCT_PTR || node->right == DESTRUCT_PTR){
        return BTREE_DEAD;
    }


    if (node->tmp != 0 && check_loop){
        return BTREE_LOOP;
    }
    return BTREE_NOERROR;
}

#ifdef BINTREE_BACK_LINK
BinTreeNode* binTreeGetRoot(BinTreeNode* node, binTreeError_t* err_ptr){
    binTreeError_t err = binTreeError_base(node);
    if(err != BTREE_NOERROR){
        if(err_ptr)
            *err_ptr = err;
        return nullptr;
    }

    if(node->up == nullptr)
        return node;

    setNodeUsedFlag(node);
    BinTreeNode* ret = binTreeGetRoot(node->up, err_ptr);
    clearNodeUsedFlag(node);
    return ret;
}
#endif

binTreeError_t binTreeBuild(BinTreeNode* node){
    binTreeError_t err = binTreeError_base(node);

    if(err != BTREE_NOERROR){
        return err;
    }
    setNodeUsedFlag(node);

    #ifdef BINTREE_STORE_SIZE
    node->size = 1;
    #endif

    int err_i = 0;
    if(node->left != nullptr){
        binTreeError_t err_s = BTREE_ERRUNK;
        err_s = binTreeBuild(node->left);

        if(err_s != BTREE_NOERROR){
            return passErrFromNode(err_s);
        }
        if(!(err_s & (BTREE_BAD | BTREE_DEAD))){
            #ifdef BINTREE_STORE_SIZE
            node->size += node->left->size;
            #endif
            #ifdef BINTREE_BACK_LINK
            node->left->up = node;
            #endif
        }
        err_i |= passErrFromNode(err_s);
    }

    if(node->right != nullptr){
        binTreeError_t err_s = BTREE_ERRUNK;
        err_s = binTreeBuild(node->right);

        if(err_s != BTREE_NOERROR){
            return passErrFromNode(err_s);
        }
        if(!(err_s & (BTREE_BAD | BTREE_DEAD))){
            #ifdef BINTREE_STORE_SIZE
            node->size += node->right->size;
            #endif
            #ifdef BINTREE_BACK_LINK
            node->right->up = node;
            #endif
        }
    }

    clearNodeUsedFlag(node);
    return BTREE_NOERROR;

}

binTreeError_t binTreeError_dwn(BinTreeNode* node, bool local){
    binTreeError_t err = binTreeError_base(node);

    if(err != BTREE_NOERROR){
        return err;
    }
    setNodeUsedFlag(node);

    size_t st_size = 1;

    int err_i = 0;
    if(node->left != nullptr){
        binTreeError_t err_s = BTREE_ERRUNK;
        if(local)
            err_s = binTreeError_base(node->left);
        else
            err_s = binTreeError_dwn(node->left, false);

        if(!(err_s & (BTREE_BAD | BTREE_DEAD))){
            #ifdef BINTREE_BACK_LINK
            st_size += node->left->size;
            if(node->left->up != node)
                err_i |= BTREE_BADEDGE;
            #endif
        }
        err_i |= passErrFromNode(err_s);
    }

    if(node->right != nullptr){
        binTreeError_t err_s = BTREE_ERRUNK;
        if(local)
            err_s = binTreeError_base(node->right);
        else
            err_s = binTreeError_dwn(node->right, false);

        if(!(err_s & (BTREE_BAD | BTREE_DEAD))){
            #ifdef BINTREE_BACK_LINK
            st_size += node->right->size;
            if(node->right->up != node)
                err_i |= BTREE_BADEDGE;
            #endif
        }
        err_i |= passErrFromNode(err_s);
    }
    #ifdef BINTREE_STORE_SIZE
    if(st_size != node->size){
        err_i |= BTREE_BADSIZE;
    }
    #endif

    clearNodeUsedFlag(node);
    return (binTreeError_t)err_i;
}

binTreeError_t binTreeError(BinTreeNode* node, bool local){
    if(local){
        return (binTreeError_t)(binTreeError_dwn(node, true) | passErrFromNode(binTreeError_base(node)));
    }
    #ifdef BINTREE_BACK_LINK
    binTreeError_t err = BTREE_NOERROR;
    BinTreeNode* root = binTreeGetRoot(node, &err);
    if(err != BTREE_NOERROR){
        return err;
    }
    return binTreeError_dwn(root);
    #else
    return binTreeError_dwn(node);
    #endif
}

static binTreeError_t binTreeError_dbg(BinTreeNode* node){
    return binTreeError(node);
}

    #define COLOR_NORM_LINE     "\"#f0f0f0\""
    #define COLOR_NORM_TXT      "\"#f0f0f0\""
    #define COLOR_NORM_FILL     "\"#25252F\""
    #define COLOR_VALID_LINE    "\"#dad09c\""
    #define COLOR_INVALID_LINE  "\"#ff3e3e\""
    #define COLOR_NOTEXIST_LINE "\"#ff3eff\""
    #define COLOR_TOBADPTR_LINE "\"#af3eff\""
    #define COLOR_LOOP_LINE     "\"#1fd0c7\""
    #define COLOR_REF_LINE     "\"#af3eff\""

static void dumpEdge(BinTreeNode* from, const char* from_port, BinTreeNode* to, FILE* file){
    fprintf(file, "N%p:<%s>->N%p[", from, from_port, to);
    binTreeError_t err = binTreeError_base(to);
    if(err == BTREE_NOERROR){
        #ifdef BINTREE_BACK_LINK
        if(to->up == from*)
            fprintf(file, "color=" COLOR_VALID_LINE ", dir = both, arrowtail = crow");
        else
            fprintf(file, "color=" COLOR_INVALID_LINE);
        #else
            fprintf(file, "color=" COLOR_VALID_LINE);
        #endif

    }
    else{
        if(err & BTREE_LOOP)
            fprintf(file, "color=" COLOR_LOOP_LINE ", constraint=false");
        else
            fprintf(file, "color=" COLOR_TOBADPTR_LINE ", style = dashed");
    }
    fprintf(file, "]\n");
}

static void binTreeDump_dwn(BinTreeNode* node, FILE* file){
    if (binTreeError_base(node) != BTREE_NOERROR){
        return;
    }
    const char* node_line_color = COLOR_NORM_LINE;
    binTreeError_t err = binTreeError(node, true);
    if (err != BTREE_NOERROR){
        node_line_color = COLOR_INVALID_LINE;
    }

    fprintf(file, "N%p[shape=plaintext, style=solid, color=%s,"
                  "label=<<TABLE  BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR = %s>\n"
                  "<TR><TD COLSPAN=\"2\">D: " , node, node_line_color, COLOR_NORM_FILL);
                  dumpElem(file, &(node->data));
    fprintf(file, " </TD></TR>\n" );
    #ifdef BINTREE_STORE_SIZE
    fprintf(file, "<TR><TD COLSPAN=\"2\">S: %lu </TD></TR>\n", node->size);
    #endif
    fprintf(file, "<TR><TD PORT=\"L\">L</TD> <TD PORT=\"R\">R</TD></TR>\n"
                  "</TABLE>> ]\n");

    setNodeUsedFlag(node);
    if(node->left){
        dumpEdge(node, "L", node->left, file);
        binTreeDump_dwn(node->left, file);
    }
    if(node->right){
        dumpEdge(node, "R", node->right, file);
        binTreeDump_dwn(node->right, file);
    }
    clearNodeUsedFlag(node);
}

void binTreeDump(BinTreeNode* refnode){
    hline_log();
    binTreeError_t err = binTreeError(refnode);
    printf_log("Tree dump\n");
    bool ptr_ok = !(err & VAR_NULL || err & VAR_BAD);

    if (err == BTREE_NOERROR){
       printf_log("    Tree ok\n");
    }
    else {
       printf_log("    ERRORS:\n");
    }

    printBaseError_log((baseError_t) err);
    if (!ptr_ok){
        hline_log();
        return;
    }
    if (err & BTREE_BADEDGE){
        printf_log("     Broken edge\n");
    }
    if (err & BTREE_BADNODE){
        printf_log("     Bad node poiter in tree\n");
    }
    if (err & BTREE_DEADNODE){
        printf_log("     Dead/uninitialised node in tree\n");
    }
    if (err & BTREE_LOOP){
        printf_log("     Loop in tree\n");
    }
    if (err & BTREE_BADSIZE){
        printf_log("     Size does not match\n");
    }

    FILE* graph_file = fopen("graph.tmp", "w");
    fprintf(graph_file, "digraph G{\n");
    fprintf(graph_file, "rankdir=TB; bgcolor=\"#151515\";\n"
                        "node[shape=diamond, style=filled, fillcolor=" COLOR_NORM_FILL ", color=" COLOR_NOTEXIST_LINE ", fontcolor=" COLOR_NORM_TXT "]\n"
                        "edge[weight=1, color=" COLOR_NORM_LINE "]\n");
    fprintf(graph_file, "ref[shape=ellipse, color = " COLOR_REF_LINE "]\n");
    fprintf(graph_file, "ref->N%p\n", refnode);

    binTreeDump_dwn(refnode, graph_file);

    fprintf(graph_file, "}");
    fclose(graph_file);

    char cmd_str[100] = "dot -Tpng graph.tmp -o";
    embedNewDumpFile(cmd_str + strlen(cmd_str), "List_dump", ".png", "img");

    system(cmd_str);
}
#ifdef BINTREE_STORE_SIZE
binTreeError_t binTreeUpdSize(BinTreeNode* node){
    binTreeError_t err = binTreeError_base(node);
    if(err != BTREE_NOERROR){
        printf_log("E1\n");
        return err;
    }

    node->size = 1;
    if(node->left){
        err = binTreeError_base(node->left, false);
        if(err == BTREE_NOERROR){
            node->size += node->left->size;
        }
        else{
            return err;
        }
    }
    if(node->right){
        err = binTreeError_base(node->right, false);
        if(err == BTREE_NOERROR){
            node->size += node->right->size;
        }
        else{
            return err;
        }
    }

    #ifdef BINTREE_BACK_LINK
    if(node->up == nullptr){
        return BTREE_NOERROR;
    }
    setNodeUsedFlag(node);
    err = binTreeUpdSize(node->up);
    clearNodeUsedFlag(node);
    #endif
    return BTREE_NOERROR;
}
#endif

binTreeError_t binTreeAttach(BinTreeNode* subtree, BinTreeNode* att_node, binTreePos_t att_pos){
    binTreeCheckRet(att_node , binTreeError_dbg(att_node));
    binTreeCheckRet(subtree  , binTreeError_dbg(subtree ));

    BinTreeNode** att_point = nullptr;
    switch(att_pos){
    case BTREE_POS_LEFT:
        att_point = &(att_node->left);
        break;
    case BTREE_POS_RIGHT:
        att_point = &(att_node->right);
        break;
    }

    if(att_point == nullptr || *att_point != nullptr){
        return BTREE_BADOP;
    }
    *att_point  = subtree;
    #ifdef BINTREE_BACK_LINK
    subtree->up = att_node;
    #endif
    subtree->usedc++;

    #ifdef BINTREE_STORE_SIZE
    binTreeUpdSize(att_node);
    #endif
    return binTreeError_dbg(att_node);
}

binTreeError_t binTreeAddNode(BinTreeNode* att_node, BINTREE_ELEM_T elem, binTreePos_t att_pos){
    BinTreeNode* new_node = (BinTreeNode*)malloc(sizeof(*new_node));
    binTreeNodeCtor(new_node, elem, 0);
    return binTreeAttach(new_node, att_node, att_pos);
}


static void binTreePrintToFile_(BinTreeNode* node, FILE* file, int layer = 0){
    #define place_tabs(_count)      \
    for(int x = 0; x < (_count); x++){ \
        fprintf(file, "  ");        \
    }

    place_tabs(layer)
    fprintf (file, "\""  );
    dumpElem(file, &(node->data));
    fprintf (file, "\"\n");
    place_tabs(layer)
    if(node->left){
        fprintf(file, "{\n");
        binTreePrintToFile_(node->left, file, layer+1);
    }
    else{
        fprintf(file, "{}\n");
    }

    place_tabs(layer)
    if(node->right){
        fprintf(file, "{\n");
        binTreePrintToFile_(node->right, file, layer+1);
    }
    else{
        fprintf(file, "{}\n");
    }

    place_tabs(layer-1)
    fprintf(file, "}\n");
    return;
}

void binTreePrintToFile(BinTreeNode* node, FILE* file){
    if(binTreeError(node, false)){
        printf("bad tree");
        return;
    }
    fprintf(file, "{\n");
    binTreePrintToFile_(node, file);
}

BinTreeNode* binTreeFind(BinTreeNode* node, BINTREE_ELEM_T elem){
    if(binTreeError_base(node) != BTREE_NOERROR){
        return nullptr;
    }

    if(elemCompare(node->data, elem) == 0){
        return node;
    }
    if(node->left){
        BinTreeNode* res = binTreeFind(node->left, elem);
        if(res != nullptr)
            return res;
    }
    if(node->right){
        BinTreeNode* res = binTreeFind(node->right, elem);
        if(res != nullptr)
            return res;
    }
    return nullptr;
}

static BinTreeNode* binTreeReadFromFile_(FILE* file){
    char c = fgetc(file);
    while(isspace(c) || iscntrl(c)){
        c = fgetc(file);
    }
    if(c == '}'){
        return nullptr;
    }

    BinTreeNode* node = (BinTreeNode*)malloc(sizeof(*node));
    binTreeNodeCtor(node, BINTREE_BADELEM);

    int next_set = 0;
    while(c != '}' && c != EOF){
        if(c == '"'){
            node->data = readElem(file);
        }
        if(c == '{'){
            BinTreeNode* new_node = binTreeReadFromFile_(file);
            switch(next_set){
            case 0:
                node->left = new_node;
                break;
            case 1:
                node->right = new_node;
                break;
            }
            next_set++;
        }

        c = fgetc(file);
    }
    if(c == EOF){
        free(node);
        return nullptr;
    }
    return node;
}

BinTreeNode* binTreeReadFromFile(FILE* file){
    char c = fgetc(file);
    while(c != EOF && c != '{'){
        c = fgetc(file);
    }
    if(c == '{'){
        BinTreeNode* ret = binTreeReadFromFile_(file);
        binTreeBuild(ret);
        return ret;
    }
    return nullptr;
}
