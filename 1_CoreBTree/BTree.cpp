#include "BTree.h"
#include <stdexcept>
#include <cassert>
BTreeNode::BTreeNode(bool isLeaf) : n(0), leaf(isLeaf) {
    for (int i = 0; i < MAX_KEYS;      ++i) keys    [i] = 0;
    for (int i = 0; i < MAX_KEYS;      ++i) values  [i] = 0;
    for (int i = 0; i < MAX_CHILDREN;  ++i) children[i] = nullptr;
}
BTree::BTree(int minDegree) : root(nullptr), t(minDegree) {
    if (t < 2) t = 2;
}

BTree::~BTree() {
    destroyTree(root);
    root = nullptr;
}

void BTree::destroyTree(BTreeNode* node) {
    if (node == nullptr) return;
    if (!node->leaf) {
        for (int i = 0; i <= node->n; ++i)
            destroyTree(node->children[i]);
    }
    delete node;
}
bool BTree::isEmpty() const {
    return (root == nullptr || root->n == 0);
}

void BTree::clear() {
    destroyTree(root);
    root = nullptr;
}
bool BTree::search(int key, int* outValue) const {
    return (searchNode(root, key, outValue) != nullptr);
}
BTreeNode* BTree::searchNode(BTreeNode* node, int key, int* outValue) const {
    if (node == nullptr) return nullptr;
    int i = 0;
    while (i < node->n && key > node->keys[i]) ++i;

    if (i < node->n && key == node->keys[i]) {
        if (outValue) *outValue = node->values[i];
        return node;
    }
    if (node->leaf) return nullptr;

    return searchNode(node->children[i], key, outValue);
}

bool BTree::update(int key, int newValue) {
    BTreeNode* node = searchNode(root, key, nullptr);
    if (node == nullptr) return false;
    int i = 0;
    while (i < node->n && key > node->keys[i]) ++i;
    node->values[i] = newValue;
    return true;
}
bool BTree::insert(int key, int value) {
    if (search(key)) return false;

    if (root == nullptr) {
        root = new BTreeNode(true);
        root->keys  [0] = key;
        root->values[0] = value;
        root->n = 1;
        return true;
    }

    if (root->n == 2 * t - 1) {
        BTreeNode* newRoot = new BTreeNode(false);
        newRoot->children[0] = root;
        root = newRoot;

        splitChild(root, 0, root->children[0]);
    }

    insertNonFull(root, key, value);
    return true;
}

void BTree::splitChild(BTreeNode* parent, int childIndex, BTreeNode* child) {
    BTreeNode* rightNode = new BTreeNode(child->leaf);
    rightNode->n = t - 1;
    for (int j = 0; j < t - 1; ++j) {
        rightNode->keys  [j] = child->keys  [j + t];
        rightNode->values[j] = child->values[j + t];
    }
    if (!child->leaf) {
        for (int j = 0; j < t; ++j)
            rightNode->children[j] = child->children[j + t];
    }
    child->n = t - 1;
    for (int j = parent->n; j >= childIndex + 1; --j)
        parent->children[j + 1] = parent->children[j];
    parent->children[childIndex + 1] = rightNode;

    for (int j = parent->n - 1; j >= childIndex; --j) {
        parent->keys  [j + 1] = parent->keys  [j];
        parent->values[j + 1] = parent->values[j];
    }
    parent->keys  [childIndex] = child->keys  [t - 1];
    parent->values[childIndex] = child->values[t - 1];
    parent->n++;
}
void BTree::insertNonFull(BTreeNode* node, int key, int value) {
    int i = node->n - 1;

    if (node->leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys  [i + 1] = node->keys  [i];
            node->values[i + 1] = node->values[i];
            --i;
        }
        node->keys  [i + 1] = key;
        node->values[i + 1] = value;
        node->n++;
    } else {
        while (i >= 0 && key < node->keys[i]) --i;
        ++i; 

        if (node->children[i]->n == 2 * t - 1) {
            splitChild(node, i, node->children[i]);
            if (key > node->keys[i]) ++i;
        }
        insertNonFull(node->children[i], key, value);
    }
}

bool BTree::remove(int key) {
    if (root == nullptr) return false;
    if (!search(key))   return false;

    deleteFromNode(root, key);

    if (root->n == 0) {
        BTreeNode* oldRoot = root;
        root = root->leaf ? nullptr : root->children[0];
        oldRoot->n = 0; 
        for (int i = 0; i < MAX_CHILDREN; ++i)
            oldRoot->children[i] = nullptr;
        delete oldRoot;
    }
    return true;
}

void BTree::deleteFromNode(BTreeNode* node, int key) {
    int idx = 0;
    while (idx < node->n && node->keys[idx] < key) ++idx;

    if (idx < node->n && node->keys[idx] == key) {
        if (node->leaf) {
            deleteFromLeaf(node, idx);
        } else {
            deleteFromInternal(node, idx);
        }
        return;
    }
    if (node->leaf) return;
    bool inLastChild = (idx == node->n);
    if (node->children[idx]->n < t) fill(node, idx);
    if (inLastChild && idx > node->n) {
        deleteFromNode(node->children[idx - 1], key);
    } else {
        deleteFromNode(node->children[idx], key);
    }
}
void BTree::deleteFromLeaf(BTreeNode* node, int idx) {
    for (int i = idx + 1; i < node->n; ++i) {
        node->keys  [i - 1] = node->keys  [i];
        node->values[i - 1] = node->values[i];
    }
    node->n--;
}
void BTree::deleteFromInternal(BTreeNode* node, int idx) {
    int key = node->keys[idx];

    if (node->children[idx]->n >= t) {
        node->keys  [idx] = getPredKey  (node, idx);
        node->values[idx] = getPredValue(node, idx);
        deleteFromNode(node->children[idx], node->keys[idx]);
    } else if (node->children[idx + 1]->n >= t) {
        node->keys  [idx] = getSuccKey  (node, idx);
        node->values[idx] = getSuccValue(node, idx);
        deleteFromNode(node->children[idx + 1], node->keys[idx]);
    } else {
        merge(node, idx);
        deleteFromNode(node->children[idx], key);
    }
}

int BTree::getPredKey(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx];
    while (!cur->leaf) cur = cur->children[cur->n];
    return cur->keys[cur->n - 1];
}

int BTree::getPredValue(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx];
    while (!cur->leaf) cur = cur->children[cur->n];
    return cur->values[cur->n - 1];
}

int BTree::getSuccKey(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx + 1];
    while (!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
}

int BTree::getSuccValue(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx + 1];
    while (!cur->leaf) cur = cur->children[0];
    return cur->values[0];
}
void BTree::fill(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->n >= t) {
        borrowFromPrev(node, idx);
    } else if (idx != node->n && node->children[idx + 1]->n >= t) {
        borrowFromNext(node, idx);
    } else {
        if (idx != node->n) {
            merge(node, idx);
        } else {
            merge(node, idx - 1);
        }
    }
}
void BTree::borrowFromPrev(BTreeNode* node, int idx) {
    BTreeNode* child  = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];
    for (int i = child->n - 1; i >= 0; --i) {
        child->keys  [i + 1] = child->keys  [i];
        child->values[i + 1] = child->values[i];
    }
    if (!child->leaf) {
        for (int i = child->n; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }
    child->keys  [0] = node->keys  [idx - 1];
    child->values[0] = node->values[idx - 1];
    if (!child->leaf)
        child->children[0] = sibling->children[sibling->n];
    node->keys  [idx - 1] = sibling->keys  [sibling->n - 1];
    node->values[idx - 1] = sibling->values[sibling->n - 1];

    child->n++;
    sibling->n--;
}

void BTree::borrowFromNext(BTreeNode* node, int idx) {
    BTreeNode* child  = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    child->keys  [child->n] = node->keys  [idx];
    child->values[child->n] = node->values[idx];
    if (!child->leaf)
        child->children[child->n + 1] = sibling->children[0];

    node->keys  [idx] = sibling->keys  [0];
    node->values[idx] = sibling->values[0];

    for (int i = 1; i < sibling->n; ++i) {
        sibling->keys  [i - 1] = sibling->keys  [i];
        sibling->values[i - 1] = sibling->values[i];
    }
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->n; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }

    child->n++;
    sibling->n--;
}
void BTree::merge(BTreeNode* node, int idx) {
    BTreeNode* leftChild  = node->children[idx];
    BTreeNode* rightChild = node->children[idx + 1];
    leftChild->keys  [t - 1] = node->keys  [idx];
    leftChild->values[t - 1] = node->values[idx];
    for (int i = 0; i < rightChild->n; ++i) {
        leftChild->keys  [i + t] = rightChild->keys  [i];
        leftChild->values[i + t] = rightChild->values[i];
    }

    if (!leftChild->leaf) {
        for (int i = 0; i <= rightChild->n; ++i)
            leftChild->children[i + t] = rightChild->children[i];
    }

    leftChild->n = 2 * t - 1;
    for (int i = idx + 1; i < node->n; ++i) {
        node->keys  [i - 1] = node->keys  [i];
        node->values[i - 1] = node->values[i];
    }
    for (int i = idx + 2; i <= node->n; ++i)
        node->children[i - 1] = node->children[i];

    node->n--;
    for (int i = 0; i < MAX_CHILDREN; ++i)
        rightChild->children[i] = nullptr;
    delete rightChild;
}

void BTree::traverseInOrder(std::function<void(int, int)> callback) const {
    traverseHelper(root, callback);
}

void BTree::traverseHelper(BTreeNode* node, std::function<void(int, int)> cb) const {
    if (node == nullptr) return;

    for (int i = 0; i < node->n; ++i) {
        if (!node->leaf) traverseHelper(node->children[i], cb);
        cb(node->keys[i], node->values[i]);
    }
    if (!node->leaf) traverseHelper(node->children[node->n], cb);
}
int BTree::getHeight() const {
    return heightHelper(root);
}

int BTree::heightHelper(BTreeNode* node) const {
    if (node == nullptr)  return 0;
    if (node->leaf)       return 1;
    return 1 + heightHelper(node->children[0]);
}

long long BTree::getNodeCount() const {
    return nodeCountHelper(root);
}

long long BTree::nodeCountHelper(BTreeNode* node) const {
    if (node == nullptr) return 0;
    long long count = 1;
    if (!node->leaf) {
        for (int i = 0; i <= node->n; ++i)
            count += nodeCountHelper(node->children[i]);
    }
    return count;
}