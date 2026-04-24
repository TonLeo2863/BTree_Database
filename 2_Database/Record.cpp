#include "Record.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>

AccountRecord::AccountRecord() : stk(0), chuThe(""), soDu(0.0), chiNhanh("") {}

AccountRecord::AccountRecord(int stk, const std::string& chuThe, double soDu, const std::string& chiNhanh)
    : stk(stk), chuThe(chuThe), soDu(soDu), chiNhanh(chiNhanh) {}

std::string AccountRecord::toString() const {
    std::ostringstream oss;
    oss << std::left
        << "STK: "     << std::setw(10) << stk
        << "| Chủ thẻ: " << std::setw(25) << chuThe
        << "| Số dư: "   << std::setw(15) << std::fixed << std::setprecision(0) << soDu
        << "| Chi nhánh: " << chiNhanh;
    return oss.str();
}

std::string AccountRecord::toCSV() const {
    auto escape = [](const std::string& s) -> std::string {
        if (s.find(',') != std::string::npos || s.find('"') != std::string::npos) {
            std::string result = "\"";
            for (char c : s) {
                if (c == '"') result += "\"\""; 
                else result += c;
            }
            result += "\"";
            return result;
        }
        return s;
    };
    std::ostringstream oss;
    oss << stk << "," << escape(chuThe) << "," << std::fixed << std::setprecision(0) << soDu << "," << escape(chiNhanh);
    return oss.str();
}

std::string transactionTypeToString(TransactionType t) {
    switch (t) {
        case TransactionType::OPEN_ACCOUNT:  return "OPEN";
        case TransactionType::CLOSE_ACCOUNT: return "CLOSE";
        case TransactionType::DEPOSIT:       return "DEPOSIT";
        case TransactionType::WITHDRAW:      return "WITHDRAW";
        case TransactionType::TRANSFER:      return "TRANSFER";
        default:                             return "UNKNOWN";
    }
}

TransactionNode::TransactionNode(TransactionType type, const AccountRecord& rec, const std::string& ts)
    : type(type), record(rec), timestamp(ts), next(nullptr) {}

TransactionHistory::TransactionHistory() : head(nullptr), tail(nullptr), size(0) {}

TransactionHistory::~TransactionHistory() { clear(); }

void TransactionHistory::clear() {
    TransactionNode* cur = head;
    while (cur != nullptr) {
        TransactionNode* next = cur->next;
        delete cur;
        cur = next;
    }
    head = nullptr;
    tail = nullptr;
    size = 0;
}

std::string TransactionHistory::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

void TransactionHistory::addTransaction(TransactionType type, const AccountRecord& record) {
    TransactionNode* newNode = new TransactionNode(type, record, getCurrentTimestamp());
    newNode->next = head;
    head = newNode;
    if (tail == nullptr) tail = head; 
    size++;
    if (size > MAX_HISTORY) removeTail();
}

void TransactionHistory::removeTail() {
    if (head == nullptr) return;
    if (head == tail) {
        delete head;
        head = tail = nullptr;
        size = 0;
        return;
    }
    TransactionNode* cur = head;
    while (cur->next != tail) cur = cur->next;
    delete tail;
    tail = cur;
    tail->next = nullptr;
    size--;
}

void TransactionHistory::printHistory(int limit) const {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                  LỊCH SỬ GIAO DỊCH (DSLK)                  ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════╣\n";

    if (head == nullptr) {
        std::cout << "║  (Chưa có giao dịch nào)                                   ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════╝\n";
        return;
    }

    TransactionNode* cur = head;
    int count = 0;
    while (cur != nullptr && count < limit) {
        std::cout << "║ [" << cur->timestamp << "] "
                  << std::left << std::setw(10) << transactionTypeToString(cur->type)
                  << " STK: " << std::setw(8) << cur->record.stk
                  << " | " << cur->record.chuThe.substr(0, 15)
                  << "\n";
        cur = cur->next;
        count++;
    }
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
}

int  TransactionHistory::getSize() const { return size; }
bool TransactionHistory::isEmpty() const { return size == 0; }

AccountDatabase::AccountDatabase() : btreeIndex(BTREE_T), recordMap(), history(), recordCount(0) {}

bool AccountDatabase::addRecord(const AccountRecord& record) {
    if (record.stk <= 0) return false; 
    if (btreeIndex.search(record.stk)) return false; 

    btreeIndex.insert(record.stk, 1);      
    recordMap[record.stk] = record;         
    recordCount++;

    history.addTransaction(TransactionType::OPEN_ACCOUNT, record);
    return true;
}

const AccountRecord* AccountDatabase::searchRecord(int stk) const {
    if (!btreeIndex.search(stk)) return nullptr;
    auto it = recordMap.find(stk);
    if (it == recordMap.end()) return nullptr;
    return &(it->second);
}

bool AccountDatabase::updateRecord(const AccountRecord& record) {
    if (!btreeIndex.search(record.stk)) return false;
    recordMap[record.stk] = record; 
    history.addTransaction(TransactionType::DEPOSIT, record); // Tạm ghi là Deposit/Update
    return true;
}

bool AccountDatabase::deleteRecord(int stk) {
    if (!btreeIndex.search(stk)) return false;
    auto it = recordMap.find(stk);
    if (it != recordMap.end()) {
        history.addTransaction(TransactionType::CLOSE_ACCOUNT, it->second);
        recordMap.erase(it);
    }
    btreeIndex.remove(stk);
    recordCount--;
    return true;
}

void AccountDatabase::forEachSorted(std::function<void(const AccountRecord&)> callback) const {
    btreeIndex.traverseInOrder([&](int key, int) {
        auto it = recordMap.find(key);
        if (it != recordMap.end()) callback(it->second);
    });
}

void AccountDatabase::rangeScan(int minStk, int maxStk, std::function<void(const AccountRecord&)> callback) const {
    btreeIndex.rangeScan(minStk, maxStk, [&](int key, int) {
        auto it = recordMap.find(key);
        if (it != recordMap.end()) callback(it->second);
    });
}

std::vector<AccountRecord> AccountDatabase::getAllRecords() const {
    std::vector<AccountRecord> result;
    result.reserve(recordCount);
    for (const auto& kv : recordMap) {
        result.push_back(kv.second);
    }
    return result;
}

int  AccountDatabase::getRecordCount() const { return recordCount; }
bool AccountDatabase::isEmpty()        const { return recordCount == 0; }

void AccountDatabase::clear() {
    btreeIndex.clear();
    recordMap.clear();
    history.clear();
    recordCount = 0;
}

TransactionHistory& AccountDatabase::getHistory() { return history; }
void AccountDatabase::printHistory(int limit) const { history.printHistory(limit); }
int  AccountDatabase::getBTreeHeight()   const { return btreeIndex.getHeight(); }
long long AccountDatabase::getBTreeNodeCount() const { return btreeIndex.getNodeCount(); }