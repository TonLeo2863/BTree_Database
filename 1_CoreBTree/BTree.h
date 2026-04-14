#pragma once
#include <functional>
#include <iostream>

static const int BTREE_T        = 100;
static const int MAX_KEYS       = 2 * BTREE_T - 1;   
static const int MIN_KEYS       = BTREE_T - 1;        
static const int MAX_CHILDREN   = 2 * BTREE_T;        

struct BTreeNode {
    int       keys    [MAX_KEYS     ];
    int       values  [MAX_KEYS     ];
    BTreeNode* children[MAX_CHILDREN];
    int       n;
    bool      leaf;

    explicit BTreeNode(bool isLeaf);
};


class BTree {
public:
    explicit BTree(int minDegree = BTREE_T);
    ~BTree();

    bool insert(int key, int value = 0);

    bool search(int key, int* outValue = nullptr) const;
    bool remove(int key);
    bool update(int key, int newValue);
    void traverseInOrder(std::function<void(int key, int value)> callback) const;
    int       getHeight()    const; 
    long long getNodeCount() const;
    bool      isEmpty()      const;
    void      clear();              

private:
    BTreeNode* root;
    int        t;  
    BTreeNode* searchNode(BTreeNode* node, int key, int* outValue) const;

    void insertNonFull(BTreeNode* node, int key, int value);
    void splitChild(BTreeNode* parent, int childIndex, BTreeNode* child);
    void deleteFromNode      (BTreeNode* node, int key);
    void deleteFromLeaf      (BTreeNode* node, int idx);
    void deleteFromInternal  (BTreeNode* node, int idx);
    int  getPredKey          (BTreeNode* node, int idx); 
    int  getPredValue        (BTreeNode* node, int idx);
    int  getSuccKey          (BTreeNode* node, int idx);
    int  getSuccValue        (BTreeNode* node, int idx);
    void fill                (BTreeNode* node, int idx); 
    void borrowFromPrev      (BTreeNode* node, int idx);
    void borrowFromNext      (BTreeNode* node, int idx);
    void merge               (BTreeNode* node, int idx);
    void      destroyTree         (BTreeNode* node);
    void      traverseHelper      (BTreeNode* node,
                                   std::function<void(int, int)> cb) const;
    int       heightHelper        (BTreeNode* node) const;
    long long nodeCountHelper     (BTreeNode* node) const;
};