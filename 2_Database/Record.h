#pragma once
#include <string>

struct TransactionNode {
    std::string date;
    std::string action; 
    int quantity;
    TransactionNode* next;
};

struct HardwareRecord {
    int partID;           
    std::string partName; 
    std::string category; 
    int stock;            
    double price;        
    TransactionNode* historyHead; 
};