#include <iostream>
#include <string>
#include <cstring>

#include "../2_Database/Record.h"
#include "../3_FileIO/FileHandler.h"
#include "../4_UI_Console/UserInterface.h"
#include "Benchmark.h"
static void printUsage(const char* programName) {
    std::cout << "\nUsage:\n";
    std::cout << "  " << programName << "                    — Interactive UI\n";
    std::cout << "  " << programName << " --benchmark         — Full scalability test (1K-1M)\n";
    std::cout << "  " << programName << " --bench-quick       — Quick benchmark (10K)\n";
    std::cout << "  " << programName << " --gen-csv N file    — Generate N rows into CSV\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc >= 2 && std::strcmp(argv[1], "--benchmark") == 0) {
        Benchmark bm;
        bm.runScalabilityTest();
        return 0;
    }

    if (argc >= 2 && std::strcmp(argv[1], "--bench-quick") == 0) {
        Benchmark bm;
        std::cout << "\n  [Quick Benchmark] n = 10,000\n";
        BenchmarkSuite suite = bm.runFullSuite(10000);
        suite.print();
        return 0;
    }

    if (argc >= 4 && std::strcmp(argv[1], "--gen-csv") == 0) {
        int n = std::atoi(argv[2]);
        std::string filename = argv[3];
        if (n <= 0) {
            std::cerr << "[Error] N must be a positive integer.\n";
            return 1;
        }
        FileResult r = FileHandler::generateSampleCSV(filename, n);
        std::cout << (r.success ? "[OK] " : "[FAIL] ") << r.message << "\n";
        return r.success ? 0 : 1;
    }

    if (argc >= 2 && (std::strcmp(argv[1], "--help") == 0 ||
                      std::strcmp(argv[1], "-h") == 0)) {
        printUsage(argv[0]);
        return 0;
    }

    ComponentDatabase db;

    if (argc >= 2) {
        std::string filename = argv[1];
        std::cout << "\n  [Startup] Đang tải dữ liệu từ [" << filename << "]...\n";
        FileResult r = FileHandler::importCSV(db, filename);
        std::cout << "  " << r.message << "\n";
    }

    UserInterface ui(db);
    ui.run();
    return 0;
}