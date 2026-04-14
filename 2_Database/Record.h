#pragma once
/**
 * ============================================================
 * MODULE: 2_Database
 * FILE  : Record.h
 * DESC  : Domain model + data management layer.
 *
 *   ComponentRecord   — struct for a warehouse component (linh kiện)
 *   TransactionNode   — node of the Singly Linked List (DSLK đơn)
 *   TransactionHistory — manages transaction log via DSLK
 *   ComponentDatabase  — combines BTree index + record storage
 *                        (this is the class other modules should use)
 *
 * Dependency: uses 1_CoreBTree via BTree.h (include path must be set)
 * ============================================================
 */

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include "../1_CoreBTree/BTree.h"

// ── Maximum history entries kept in linked list ─────────────
static const int MAX_HISTORY = 1000;

// ============================================================
// ComponentRecord — one row in the warehouse database
// ============================================================
struct ComponentRecord {
    int         id;             // Primary key (component ID)
    std::string name;           // e.g. "Intel Core i9-13900K"
    std::string category;       // e.g. "CPU", "RAM", "GPU"
    int         quantity;       // units in stock
    double      price;          // price in VND (or USD)
    std::string supplier;       // supplier name

    // Default constructor
    ComponentRecord();

    // Convenience constructor
    ComponentRecord(int id,
                    const std::string& name,
                    const std::string& category,
                    int quantity,
                    double price,
                    const std::string& supplier = "");

    // Formatted string for display
    std::string toString() const;

    // CSV row (no header)
    std::string toCSV() const;
};

// ============================================================
// TransactionRecord — what kind of operation happened
// ============================================================
enum class TransactionType {
    INSERT,
    UPDATE,
    DELETE,
    SEARCH     // log reads too (optional)
};

std::string transactionTypeToString(TransactionType t);

// ============================================================
// TransactionNode — node in the Singly Linked List
// ============================================================
struct TransactionNode {
    TransactionType   type;
    ComponentRecord   record;    // snapshot of record at time of operation
    std::string       timestamp; // wall-clock string
    TransactionNode*  next;      // pointer to next node (DSLK đơn)

    TransactionNode(TransactionType type,
                    const ComponentRecord& rec,
                    const std::string& ts);
};

// ============================================================
// TransactionHistory — Singly Linked List of transactions
// ============================================================
/**
 * TransactionHistory implements a DSLK đơn (singly linked list).
 * - head: newest entry (push_front is O(1))
 * - tail: oldest entry (kept to detect size > MAX_HISTORY)
 * - size: current count of nodes
 *
 * When size exceeds MAX_HISTORY, the oldest entry (tail) is dropped.
 * This bounds memory usage to O(MAX_HISTORY).
 */
class TransactionHistory {
public:
    TransactionHistory();
    ~TransactionHistory();  // frees all nodes — no memory leak

    // Add a new transaction entry (O(1) push_front)
    void addTransaction(TransactionType type, const ComponentRecord& record);

    // Display the last 'limit' entries to stdout
    void printHistory(int limit = 20) const;

    // Clear entire history
    void clear();

    // Accessors
    int  getSize()  const;
    bool isEmpty()  const;

private:
    TransactionNode* head;     // newest → oldest direction
    TransactionNode* tail;     // oldest node (used for capped eviction)
    int              size;

    // Get current time as formatted string
    static std::string getCurrentTimestamp();

    // Remove the oldest node (from tail)
    void removeTail();
};

// ============================================================
// ComponentDatabase — main facade used by all other modules
// ============================================================
/**
 * ComponentDatabase wraps:
 *   - BTree         : index for O(log n) insert/search/delete
 *   - unordered_map : O(1) direct record access by ID
 *   - TransactionHistory : DSLK of recent operations
 *
 * Other modules should depend ONLY on this class (not BTree directly).
 *
 * Design: BTree stores (id → 1) as a presence index.
 *         Actual record data lives in recordMap.
 *         This mirrors how real DBs separate index from heap file.
 */
class ComponentDatabase {
public:
    ComponentDatabase();
    ~ComponentDatabase() = default; // BTree + unordered_map self-clean

    // ── CRUD operations ─────────────────────────────────────
    /**
     * addRecord — inserts into BTree + recordMap.
     * Returns false if ID already exists.
     */
    bool addRecord(const ComponentRecord& record);

    /**
     * searchRecord — O(log n) via BTree, then O(1) map lookup.
     * Returns nullptr if not found.
     */
    const ComponentRecord* searchRecord(int id) const;

    /**
     * updateRecord — updates existing record.
     * Returns false if ID not found.
     */
    bool updateRecord(const ComponentRecord& record);

    /**
     * deleteRecord — removes from BTree + recordMap.
     * Returns false if ID not found.
     */
    bool deleteRecord(int id);

    // ── Traversal ───────────────────────────────────────────
    /**
     * forEachSorted — visits all records in ascending ID order.
     * Uses BTree's in-order traversal.
     */
    void forEachSorted(std::function<void(const ComponentRecord&)> callback) const;

    /**
     * getAllRecords — returns all records (unordered).
     * Used by FileIO for CSV export.
     */
    std::vector<ComponentRecord> getAllRecords() const;

    // ── Utility ─────────────────────────────────────────────
    int  getRecordCount() const;
    bool isEmpty()        const;
    void clear();               // removes all records + resets history

    // ── History access ──────────────────────────────────────
    TransactionHistory& getHistory();
    void printHistory(int limit = 20) const;

    // ── BTree stats (for benchmark) ─────────────────────────
    int       getBTreeHeight()    const;
    long long getBTreeNodeCount() const;

private:
    BTree                                    btreeIndex;
    std::unordered_map<int, ComponentRecord> recordMap;
    TransactionHistory                       history;
    int                                      recordCount;
};