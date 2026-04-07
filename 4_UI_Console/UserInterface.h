#pragma once
#include "BTree.h"

class UserInterface {
public:
    static void showMainMenu();
    static void handleUserInput(BTree& tree);
    static void printProductDetails(ProductRecord* record);
    static void drawTreeConsole(BTreeNode* root, int level);
};