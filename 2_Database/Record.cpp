/**
 * ============================================================
 * MODULE: 2_Database
 * FILE  : Record.cpp
 * ============================================================
 */
#include "Record.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <stdexcept>

// ============================================================
// ComponentRecord — implementation
// ============================================================
ComponentRecord::ComponentRecord()
    : id(0), name(""), category(""), quantity(0), price(0.0), supplier("") {}

ComponentRecord::ComponentRecord(int id,
                                 const std::string& name,
                                 const std::string& category,
                                 int quantity,
                                 double price,
                                 const std::string& supplier)
    : id(id), name(name), category(category),
      quantity(quantity), price(price), supplier(supplier) {}

std::string ComponentRecord::toString() const {
    std::ostringstream oss;
    oss << std::left
        << "ID: "       << std::setw(8)  << id
        << "| Name: "   << std::setw(30) << name
        << "| Cat: "    << std::setw(10) << category
        << "| Qty: "    << std::setw(8)  << quantity
        << "| Price: "  << std::setw(12) << std::fixed << std::setprecision(2) << price
        << "| Supplier: " << supplier;
    return oss.str();
}

std::string ComponentRecord::toCSV() const {
    // Escape commas in string fields by wrapping in quotes
    auto escape = [](const std::string& s) -> std::string {
        if (s.find(',') != std::string::npos ||
            s.find('"') != std::string::npos) {
            std::string result = "\"";
            for (char c : s) {
                if (c == '"') result += "\"\""; // double-quote escape
                else result += c;
            }
            result += "\"";
            return result;
        }
        return s;
    };

    std::ostringstream oss;
    oss << id << ","
        << escape(name) << ","
        << escape(category) << ","
        << quantity << ","
        << std::fixed << std::setprecision(2) << price << ","
        << escape(supplier);
    return oss.str();
}

// ── TransactionType helper ───────────────────────────────────
std::string transactionTypeToString(TransactionType t) {
    switch (t) {
        case TransactionType::INSERT: return "INSERT";
        case TransactionType::UPDATE: return "UPDATE";
        case TransactionType::DELETE: return "DELETE";
        case TransactionType::SEARCH: return "SEARCH";
        default:                      return "UNKNOWN";
    }
}

// ============================================================
// TransactionNode — constructor
// ============================================================
TransactionNode::TransactionNode(TransactionType type,
                                 const ComponentRecord& rec,
                                 const std::string& ts)
    : type(type), record(rec), timestamp(ts), next(nullptr) {}

// ============================================================
// TransactionHistory — constructor / destructor
// ============================================================
TransactionHistory::TransactionHistory()
    : head(nullptr), tail(nullptr), size(0) {}

TransactionHistory::~TransactionHistory() {
    clear();
}

void TransactionHistory::clear() {
    // Walk the list and free every node — prevents memory leak
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

// ── Static helper: current timestamp string ─────────────────
std::string TransactionHistory::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    // Format: YYYY-MM-DD HH:MM:SS
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

// ── addTransaction — O(1) push_front ────────────────────────
void TransactionHistory::addTransaction(TransactionType type,
                                        const ComponentRecord& record) {
    TransactionNode* newNode = new TransactionNode(type, record,
                                                   getCurrentTimestamp());
    newNode->next = head;
    head = newNode;

    if (tail == nullptr) tail = head; // first node

    size++;

    // Cap memory: if over limit, drop oldest (tail)
    if (size > MAX_HISTORY) removeTail();
}

// ── removeTail — O(n) but called rarely (only when > MAX_HISTORY) ─
void TransactionHistory::removeTail() {
    if (head == nullptr) return;
    if (head == tail) {
        // Only one node
        delete head;
        head = nullptr;
        tail = nullptr;
        size = 0;
        return;
    }

    // Walk to node before tail
    TransactionNode* cur = head;
    while (cur->next != tail) cur = cur->next;

    delete tail;
    tail = cur;
    tail->next = nullptr;
    size--;
}

// ── printHistory ─────────────────────────────────────────────
void TransactionHistory::printHistory(int limit) const {
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║              TRANSACTION HISTORY                     ║\n";
    std::cout << "╠══════════════════════════════════════════════════════╣\n";

    if (head == nullptr) {
        std::cout << "║  (no transactions recorded)                          ║\n";
        std::cout << "╚══════════════════════════════════════════════════════╝\n";
        return;
    }

    TransactionNode* cur = head;
    int count = 0;
    while (cur != nullptr && count < limit) {
        std::cout << "║ [" << cur->timestamp << "] "
                  << std::left << std::setw(7)
                  << transactionTypeToString(cur->type)
                  << " ID=" << cur->record.id
                  << " " << cur->record.name.substr(0, 20)
                  << "\n";
        cur = cur->next;
        count++;
    }

    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    std::cout << "  (showing " << count << " of " << size << " entries)\n\n";
}

int  TransactionHistory::getSize()  const { return size; }
bool TransactionHistory::isEmpty()  const { return size == 0; }

// ============================================================
// ComponentDatabase — implementation
// ============================================================
ComponentDatabase::ComponentDatabase()
    : btreeIndex(BTREE_T), recordMap(), history(), recordCount(0) {}

// ── addRecord ────────────────────────────────────────────────
bool ComponentDatabase::addRecord(const ComponentRecord& record) {
    if (record.id <= 0) return false; // invalid ID
    if (btreeIndex.search(record.id)) return false; // duplicate

    btreeIndex.insert(record.id, 1);      // insert into index
    recordMap[record.id] = record;         // store record data
    recordCount++;

    history.addTransaction(TransactionType::INSERT, record);
    return true;
}

// ── searchRecord ─────────────────────────────────────────────
const ComponentRecord* ComponentDatabase::searchRecord(int id) const {
    if (!btreeIndex.search(id)) return nullptr;

    auto it = recordMap.find(id);
    if (it == recordMap.end()) return nullptr;
    return &(it->second);
}

// ── updateRecord ─────────────────────────────────────────────
bool ComponentDatabase::updateRecord(const ComponentRecord& record) {
    if (!btreeIndex.search(record.id)) return false;

    recordMap[record.id] = record; // overwrite in-place
    history.addTransaction(TransactionType::UPDATE, record);
    return true;
}

// ── deleteRecord ─────────────────────────────────────────────
bool ComponentDatabase::deleteRecord(int id) {
    if (!btreeIndex.search(id)) return false;

    // Log before deletion (so we have the record data)
    auto it = recordMap.find(id);
    if (it != recordMap.end()) {
        history.addTransaction(TransactionType::DELETE, it->second);
        recordMap.erase(it);
    }

    btreeIndex.remove(id);
    recordCount--;
    return true;
}

// ── forEachSorted ────────────────────────────────────────────
void ComponentDatabase::forEachSorted(
        std::function<void(const ComponentRecord&)> callback) const {
    btreeIndex.traverseInOrder([&](int key, int /*value*/) {
        auto it = recordMap.find(key);
        if (it != recordMap.end()) callback(it->second);
    });
}

// ── getAllRecords ─────────────────────────────────────────────
std::vector<ComponentRecord> ComponentDatabase::getAllRecords() const {
    std::vector<ComponentRecord> result;
    result.reserve(recordCount);
    for (const auto& kv : recordMap) {
        result.push_back(kv.second);
    }
    return result;
}

// ── Utility ──────────────────────────────────────────────────
int  ComponentDatabase::getRecordCount() const { return recordCount; }
bool ComponentDatabase::isEmpty()        const { return recordCount == 0; }

void ComponentDatabase::clear() {
    btreeIndex.clear();
    recordMap.clear();
    history.clear();
    recordCount = 0;
}

// ── History ──────────────────────────────────────────────────
TransactionHistory& ComponentDatabase::getHistory() { return history; }

void ComponentDatabase::printHistory(int limit) const {
    history.printHistory(limit);
}

// ── BTree stats ──────────────────────────────────────────────
int       ComponentDatabase::getBTreeHeight()    const {
    return btreeIndex.getHeight();
}
long long ComponentDatabase::getBTreeNodeCount() const {
    return btreeIndex.getNodeCount();
}