#include "Benchmark.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>
#include <numeric>
using Clock    = std::chrono::high_resolution_clock;
using Ms       = std::chrono::duration<double, std::milli>;
using TimePoint = Clock::time_point;

static double elapsedMs(TimePoint t0, TimePoint t1) {
    return Ms(t1 - t0).count();
}

BenchmarkResult::BenchmarkResult()
    : operationName(""), n(0), totalMs(0), perOpMicros(0),
      btreeHeight(0), btreeNodes(0) {}

BenchmarkResult::BenchmarkResult(const std::string& name, int n,
                                  double ms, int height, long long nodes)
    : operationName(name), n(n), totalMs(ms),
      perOpMicros((n > 0) ? (ms * 1000.0 / n) : 0.0),
      btreeHeight(height), btreeNodes(nodes) {}

std::string BenchmarkResult::toTableRow() const {
    std::ostringstream oss;
    oss << "  │ " << std::left  << std::setw(22) << operationName
        << "│ " << std::right << std::setw(9) << n
        << " │ " << std::setw(11) << std::fixed << std::setprecision(2) << totalMs
        << " │ " << std::setw(11) << std::setprecision(3) << perOpMicros
        << " │ " << std::setw(7) << btreeHeight
        << " │";
    return oss.str();
}

void BenchmarkSuite::print() const {
    std::cout << "\n  ┌─────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │ BENCHMARK SUITE  n = " << std::left << std::setw(40) << n << "│\n";
    std::cout << "  ├──────────────────────┬──────────┬────────────┬────────────┬────────┤\n";
    std::cout << "  │ Operation            │    N     │  Total(ms) │  Per(µs)   │ Height │\n";
    std::cout << "  ├──────────────────────┼──────────┼────────────┼────────────┼────────┤\n";
    for (const auto& r : results) {
        std::cout << r.toTableRow() << "\n";
    }
    std::cout << "  └──────────────────────┴──────────┴────────────┴────────────┴────────┘\n";
}

std::vector<ComponentRecord> Benchmark::generateRecords(int n) {
    const char* categories[] = {"CPU","RAM","GPU","SSD","HDD",
                                  "Motherboard","PSU","Cooling"};
    const char* suppliers[]  = {"Intel","AMD","Samsung","WD","Seagate",
                                  "ASUS","MSI","Gigabyte"};
    const int catC = 8, supC = 8;

    std::vector<ComponentRecord> recs;
    recs.reserve(n);

    for (int i = 1; i <= n; ++i) {
        recs.emplace_back(
            i,
            std::string(categories[i % catC]) + "_" + std::to_string(i),
            categories[i % catC],
            10 + (i * 7) % 490,
            100.0 + i * 1.3,
            suppliers[i % supC]
        );
    }
    return recs;
}

std::vector<int> Benchmark::generateRandomIDs(int n) {
    std::vector<int> ids(n);
    std::iota(ids.begin(), ids.end(), 1);
    std::mt19937 rng(42); 
    std::shuffle(ids.begin(), ids.end(), rng);
    return ids;
}

BenchmarkResult Benchmark::runInsert(int n) {
    ComponentDatabase db;
    auto records = generateRecords(n);

    auto t0 = Clock::now();
    for (const auto& rec : records) db.addRecord(rec);
    auto t1 = Clock::now();

    return BenchmarkResult("INSERT " + std::to_string(n), n,
                           elapsedMs(t0, t1),
                           db.getBTreeHeight(), db.getBTreeNodeCount());
}

BenchmarkResult Benchmark::runSearchSequential(ComponentDatabase& db, int n) {
    volatile int found = 0; 
    auto t0 = Clock::now();
    for (int i = 1; i <= n; ++i) {
        if (db.searchRecord(i)) found++;
    }
    auto t1 = Clock::now();

    return BenchmarkResult("SEARCH seq " + std::to_string(n), n,
                           elapsedMs(t0, t1),
                           db.getBTreeHeight(), db.getBTreeNodeCount());
}

BenchmarkResult Benchmark::runSearchRandom(ComponentDatabase& db, int n) {
    auto ids = generateRandomIDs(n);
    volatile int found = 0;

    auto t0 = Clock::now();
    for (int id : ids) {
        if (db.searchRecord(id)) found++;
    }
    auto t1 = Clock::now();

    return BenchmarkResult("SEARCH rnd " + std::to_string(n), n,
                           elapsedMs(t0, t1),
                           db.getBTreeHeight(), db.getBTreeNodeCount());
}

BenchmarkResult Benchmark::runDelete(ComponentDatabase& db, int n) {
    int half = n / 2;

    auto t0 = Clock::now();
    for (int i = 1; i <= half; ++i) db.deleteRecord(i);
    auto t1 = Clock::now();

    return BenchmarkResult("DELETE n/2 " + std::to_string(half), half,
                           elapsedMs(t0, t1),
                           db.getBTreeHeight(), db.getBTreeNodeCount());
}

BenchmarkSuite Benchmark::runFullSuite(int n) {
    BenchmarkSuite suite(n);

    ComponentDatabase db;
    auto records = generateRecords(n);

    std::cout << "  [Benchmark] Đang chạy INSERT " << n << " records...\n";
    auto t0 = Clock::now();
    for (const auto& rec : records) db.addRecord(rec);
    auto t1 = Clock::now();
    suite.results.emplace_back("INSERT", n, elapsedMs(t0, t1),
                               db.getBTreeHeight(), db.getBTreeNodeCount());

    std::cout << "  [Benchmark] Đang chạy SEARCH sequential " << n << "...\n";
    suite.results.push_back(runSearchSequential(db, n));

    std::cout << "  [Benchmark] Đang chạy SEARCH random " << n << "...\n";
    suite.results.push_back(runSearchRandom(db, n));

    std::cout << "  [Benchmark] Đang chạy DELETE n/2 = " << n/2 << "...\n";
    suite.results.push_back(runDelete(db, n));

    return suite;
}

void Benchmark::runScalabilityTest() {
    const std::vector<int> scales = {1000, 10000, 100000, 1000000};

    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║       SCALABILITY TEST — B-Tree Performance Analysis        ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║  Degree t=" << BTREE_T
              << "  |  Max keys/node=" << MAX_KEYS
              << "  |  Expected height ~3 at 1M      ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";

    std::vector<BenchmarkSuite> allSuites;

    for (int n : scales) {
        std::cout << "\n  ━━━ N = " << n << " ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        BenchmarkSuite suite = runFullSuite(n);
        suite.print();
        allSuites.push_back(std::move(suite));
    }

    std::cout << "\n  ┌─── INSERT Scalability Summary ──────────────────────────────┐\n";
    std::cout << "  │     N       │ Total(ms)  │ Per op(µs) │ Tree Height │\n";
    std::cout << "  ├─────────────┼────────────┼────────────┼─────────────┤\n";
    for (const auto& s : allSuites) {
        if (!s.results.empty()) {
            const auto& r = s.results[0];
            std::cout << "  │ " << std::right << std::setw(11) << s.n
                      << " │ " << std::setw(10) << std::fixed << std::setprecision(2) << r.totalMs
                      << " │ " << std::setw(10) << std::setprecision(4) << r.perOpMicros
                      << " │ " << std::setw(11) << r.btreeHeight
                      << " │\n";
        }
    }
    std::cout << "  └─────────────┴────────────┴────────────┴─────────────┘\n";

    std::cout << "\n  ✓ Benchmark hoàn tất. Mọi bộ nhớ đã được giải phóng.\n\n";
}