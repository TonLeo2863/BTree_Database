#pragma once
#include <string>
#include <vector>
#include "../2_Database/Record.h"
#include "../3_FileIO/FileHandler.h"

struct BenchmarkResult {
    std::string operationName;
    int         n;               
    double      totalMs;         
    double      perOpMicros;     
    int         btreeHeight;     
    long long   btreeNodes;      

    BenchmarkResult();
    BenchmarkResult(const std::string& name, int n, double ms,
                    int height, long long nodes);

    std::string toTableRow() const;
};

struct BenchmarkSuite {
    int                          n;
    std::vector<BenchmarkResult> results;

    BenchmarkSuite(int n) : n(n) {}
    void print() const;
};

class Benchmark {
public:
    Benchmark() = default;
    ~Benchmark() = default;
    BenchmarkResult runInsert(int n);
    BenchmarkResult runSearchSequential(ComponentDatabase& db, int n);
    BenchmarkResult runSearchRandom(ComponentDatabase& db, int n);
    BenchmarkResult runDelete(ComponentDatabase& db, int n);
    BenchmarkSuite runFullSuite(int n);
    void runScalabilityTest();
    static void printTableHeader();
    static void printSeparator();
    static void printSuiteComparison(const std::vector<BenchmarkSuite>& suites);

private:
    static std::vector<ComponentRecord> generateRecords(int n);

    static std::vector<int> generateRandomIDs(int n);
};