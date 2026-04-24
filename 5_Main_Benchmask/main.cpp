#include <iostream>
#include <string>
#include <cstring>
#include "../2_Database/Record.h"
#include "../3_FileIO/FileHandler.h"
#include "../4_UI_Console/UserInterface.h"
#include "Benchmark.h"

int main(int argc, char* argv[]) {
    if (argc >= 2 && std::strcmp(argv[1], "--benchmark") == 0) {
        Benchmark bm;
        bm.runScalabilityTest();
        return 0;
    }

    AccountDatabase db;
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