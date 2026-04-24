#pragma once
#include <string>
#include <vector>
#include "../2_Database/Record.h"
#include "../3_FileIO/FileHandler.h"
#include "../1_CoreBTree/BST.h"

struct BenchmarkResult {
    std::string operationName;
    int         n;              
    double      totalMs;        
    int         btreeHeight;    
    int         bstHeight;      // MỚI: Chiều cao BST

    BenchmarkResult();
    BenchmarkResult(const std::string& name, int n, double ms, int btHeight, int bstHeight);
    std::string toTableRow() const;
};

struct BenchmarkSuite {
    int n;
    std::vector<BenchmarkResult> results;
    BenchmarkSuite(int n) : n(n) {}
    void print() const;
};

class Benchmark {
public:
    Benchmark() = default;
    ~Benchmark() = default;
    void runScalabilityTest();
private:
    static std::vector<AccountRecord> generateRecords(int n);
};