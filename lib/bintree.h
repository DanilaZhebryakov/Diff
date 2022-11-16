#ifndef BINTREE_H_INCLUDED
#define BINTREE_H_INCLUDED
#include "lib/debug_utils.h"

#include "formule_utils.h"

#define BINTREE_ELEM_T  MathData
#define BINTREE_BADELEM BAD_MATH_DATA
//#define BINTREE_ELEM_SPEC "s"

enum binTreeError_t{
    BASE_ERRORS(BTREE)

    BTREE_BADSIZE   = 1 << 8,
    BTREE_BADEDGE   = 1 << 9,
    BTREE_LOOP      = 1 << 10,
    BTREE_BADNODE   = 1 << 11,
    BTREE_DEADNODE  = 1 << 12

};


enum binTreePos_t{
    BTREE_POS_LEFT  = 0,
    BTREE_POS_RIGHT = 1
};

struct BinTreeNode{
    BinTreeNode* left;
    BinTreeNode* right;
    BINTREE_ELEM_T data;
    uint8_t tmp;
    int usedc;
};

/*
struct tree_iterator{
    tree_node* ptr;

};
*/

void binTreeNodeCtor(BinTreeNode* node, BINTREE_ELEM_T elem, int use = 1);

BinTreeNode* binTreeNewNode(BINTREE_ELEM_T elem, int use = 1);

void binTreeNodeDtor(BinTreeNode* node);

void binTreeDtor(BinTreeNode* node);

binTreeError_t binTreeError_base(BinTreeNode* node, bool check_loop = true);

BinTreeNode* binTreeGetRoot(BinTreeNode* node, binTreeError_t* err_ptr);

binTreeError_t binTreeError_dwn(BinTreeNode* node, bool local = false);

binTreeError_t binTreeError(BinTreeNode* node, bool local = false);

void binTreeDump(BinTreeNode* refnode);

binTreeError_t binTreeUpdSize(BinTreeNode* node);

binTreeError_t binTreeAttach(BinTreeNode* subtree, BinTreeNode* att_node, binTreePos_t att_pos);

binTreeError_t binTreeAddNode(BinTreeNode* att_node, BINTREE_ELEM_T elem, binTreePos_t att_pos);

BinTreeNode* binTreeFind(BinTreeNode* node, BINTREE_ELEM_T elem);

void binTreePrintToFile(BinTreeNode* node, FILE* file);

BinTreeNode* binTreeReadFromFile(FILE* file);


#endif // BINTREE_H_INCLUDED
