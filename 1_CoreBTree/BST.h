#pragma once
#include <algorithm>

struct BSTNode {
    int key;
    BSTNode* left;
    BSTNode* right;
    BSTNode(int k) : key(k), left(nullptr), right(nullptr) {}
};

class BST {
private:
    BSTNode* root;

    BSTNode* insertHelper(BSTNode* node, int key) {
        if (node == nullptr) return new BSTNode(key);
        if (key < node->key) node->left = insertHelper(node->left, key);
        else if (key > node->key) node->right = insertHelper(node->right, key);
        return node;
    }

    int heightHelper(BSTNode* node) const {
        if (node == nullptr) return 0;
        return 1 + std::max(heightHelper(node->left), heightHelper(node->right));
    }

    void destroyHelper(BSTNode* node) {
        if (node != nullptr) {
            destroyHelper(node->left);
            destroyHelper(node->right);
            delete node;
        }
    }

public:
    BST() : root(nullptr) {}
    ~BST() { destroyHelper(root); }
    void insert(int key) { root = insertHelper(root, key); }
    int getHeight() const { return heightHelper(root); }
};