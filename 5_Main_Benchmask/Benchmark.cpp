#include "Benchmark.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::duration<double, std::milli>;

BenchmarkResult::BenchmarkResult() : operationName(""), n(0), totalMs(0), btreeHeight(0), bstHeight(0) {}

BenchmarkResult::BenchmarkResult(const std::string& name, int n, double ms, int btHeight, int bstHeight)
    : operationName(name), n(n), totalMs(ms), btreeHeight(btHeight), bstHeight(bstHeight) {}

std::string BenchmarkResult::toTableRow() const {
    std::ostringstream oss;
    oss << "  │ " << std::left  << std::setw(22) << operationName
        << "│ " << std::right << std::setw(9) << n
        << " │ " << std::setw(11) << std::fixed << std::setprecision(2) << totalMs
        << " │ " << std::setw(12) << btreeHeight
        << " │ " << std::setw(10) << bstHeight << " │";
    return oss.str();
}

void BenchmarkSuite::print() const {
    std::cout << "\n  ┌────────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │ ĐUA CHIỀU CAO & HIỆU NĂNG: N = " << std::left << std::setw(39) << n << " │\n";
    std::cout << "  ├──────────────────────┬──────────┬────────────┬──────────────┬────────────┤\n";
    std::cout << "  │ Operation            │    N     │ Total(ms)  │ BTree Height │ BST Height │\n";
    std::cout << "  ├──────────────────────┼──────────┼────────────┼──────────────┼────────────┤\n";
    for (const auto& r : results) {
        std::cout << r.toTableRow() << "\n";
    }
    std::cout << "  └──────────────────────┴──────────┴────────────┴──────────────┴────────────┘\n";
}

std::vector<AccountRecord> Benchmark::generateRecords(int n) {
    std::vector<AccountRecord> recs;
    recs.reserve(n);
    for (int i = 1; i <= n; ++i) {
        recs.emplace_back(i, "KhachHang_" + std::to_string(i), 1000000.0, "ChiNhanh");
    }
    return recs;
}

void Benchmark::runScalabilityTest() {
    const std::vector<int> scales = {10000, 50000, 100000}; // Chạy tới 100k theo yêu cầu đề bài

    std::cout << "\n";
    std::cout << "  ╔════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║     CHỨC NĂNG NÂNG CAO: BIG DATA SIMULATION - SO SÁNH B-TREE VÀ BST    ║\n";
    std::cout << "  ╚════════════════════════════════════════════════════════════════════════╝\n\n";

    for (int n : scales) {
        BenchmarkSuite suite(n);
        AccountDatabase db;
        BST testBst;
        auto records = generateRecords(n);

        std::cout << "  [Benchmark] Đang nạp " << n << " tài khoản vào B-Tree và BST...\n";
        auto t0 = Clock::now();
        for (const auto& rec : records) {
            db.addRecord(rec);
            testBst.insert(rec.stk);
        }
        auto t1 = Clock::now();
        double ms = Ms(t1 - t0).count();

        suite.results.emplace_back("INSERT_AND_BUILD", n, ms, db.getBTreeHeight(), testBst.getHeight());
        suite.print();
        
        std::cout << "  => KẾT LUẬN: Với " << n << " dữ liệu, B-Tree (bậc 5) chỉ cao " << db.getBTreeHeight() 
                  << " tầng,\n     trong khi BST bị thoái hóa kéo dài tới " << testBst.getHeight() << " tầng.\n";
        std::cout << "  => Việc tìm kiếm trên B-Tree chỉ tốn tối đa " << db.getBTreeHeight() 
                  << " lần đọc ổ đĩa (Disk I/O).\n";
    }
    std::cout << "\n  ✓ Benchmark hoàn tất.\n\n";
}