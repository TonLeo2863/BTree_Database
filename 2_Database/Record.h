#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include "../1_CoreBTree/BTree.h"

static const int MAX_HISTORY = 1000;

struct AccountRecord {
    int         stk;          
    std::string chuThe;       
    double      soDu;         
    std::string chiNhanh;     

    AccountRecord();
    AccountRecord(int stk, const std::string& chuThe, double soDu, const std::string& chiNhanh);

    std::string toString() const;
    std::string toCSV() const;
};

enum class TransactionType {
    OPEN_ACCOUNT,
    CLOSE_ACCOUNT,
    DEPOSIT,
    WITHDRAW,
    TRANSFER
};

std::string transactionTypeToString(TransactionType t);

struct TransactionNode {
    TransactionType type;
    AccountRecord   record;    
    std::string     timestamp; 
    TransactionNode* next;     

    TransactionNode(TransactionType type, const AccountRecord& rec, const std::string& ts);
};

class TransactionHistory {
public:
    TransactionHistory();
    ~TransactionHistory();

    void addTransaction(TransactionType type, const AccountRecord& record);
    void printHistory(int limit = 20) const;
    void clear();
    int  getSize() const;
    bool isEmpty() const;

private:
    TransactionNode* head; 
    TransactionNode* tail; 
    int              size;
    static std::string getCurrentTimestamp();
    void removeTail();
};

class AccountDatabase {
public:
    AccountDatabase();
    ~AccountDatabase() = default;

    bool addRecord(const AccountRecord& record);
    const AccountRecord* searchRecord(int stk) const;
    bool updateRecord(const AccountRecord& record);
    bool deleteRecord(int stk);

    void forEachSorted(std::function<void(const AccountRecord&)> callback) const;
    void rangeScan(int minStk, int maxStk, std::function<void(const AccountRecord&)> callback) const;
    std::vector<AccountRecord> getAllRecords() const;

    int  getRecordCount() const;
    bool isEmpty()        const;
    void clear();         

    TransactionHistory& getHistory();
    void printHistory(int limit = 20) const;

    int       getBTreeHeight()   const;
    long long getBTreeNodeCount() const;

private:
    BTree                                  btreeIndex;
    std::unordered_map<int, AccountRecord> recordMap;
    TransactionHistory                     history;
    int                                    recordCount;
};