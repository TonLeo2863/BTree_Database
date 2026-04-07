#pragma once
#include "Record.h"
const int MAX_KEYS = 4; 
const int MIN_KEYS = 2; 
struct BTreeNode {
    int keys[MAX_KEYS];               
    ProductRecord* data[MAX_KEYS];    
    BTreeNode* children[MAX_KEYS + 1];
    int numKeys;
    bool isLeaf;
};

class BTree {
private:
    BTreeNode* root;
    void splitChild(BTreeNode* parent, int i, BTreeNode* fullChild);
    void insertNonFull(BTreeNode* node, int key, ProductRecord* data);
    void removeInternal(BTreeNode* node, int key);
public:
    BTree();
    void insert(int key, ProductRecord* data);  
    ProductRecord* search(int key);             
    void remove(int key);                     
};