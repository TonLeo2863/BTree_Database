#pragma once
#include "BTree.h"
#include <string>

class FileHandler {
public:
    static bool loadDataFromFile(const std::string& filename, BTree& tree);

    static bool backupData(const std::string& filename, BTree& tree);
};