#include "UserInterface.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <chrono>

#ifdef _WIN32
  #define CLEAR_CMD "cls"
#else
  #define CLEAR_CMD "clear"
#endif

UserInterface::UserInterface(ComponentDatabase& db) : db(db) {}

void UserInterface::run() {
    printBanner();

    bool running = true;
    while (running) {
        printMenu();
        int choice = readIntOption("Chọn chức năng");

        switch (choice) {
            case 1:  handleAddRecord();        break;
            case 2:  handleSearchRecord();     break;
            case 3:  handleUpdateRecord();     break;
            case 4:  handleDeleteRecord();     break;
            case 5:  handleListAll();          break;
            case 6:  handleImportCSV();        break;
            case 7:  handleExportCSV();        break;
            case 8:  handleViewHistory();      break;
            case 9:  handleStatistics();       break;
            case 10: handleQuickBenchmark();   break;
            case 0:
                if (confirmAction("Bạn có chắc muốn thoát?")) {
                    std::cout << "\n  Tạm biệt! Hệ thống đã đóng.\n\n";
                    running = false;
                }
                break;
            default:
                std::cout << "\n  [!] Lựa chọn không hợp lệ. Vui lòng thử lại.\n";
                pause();
        }
    }
}
void UserInterface::printBanner() const {
    clearScreen();
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║      KHO LINH KIỆN MÁY TÍNH — B-TREE INDEX          ║\n";
    std::cout << "  ║     Mô phỏng chỉ mục dữ liệu bằng B-Tree            ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";
}

void UserInterface::printMenu() const {
    std::cout << "\n";
    printDivider('-');
    std::cout << "  [1] Thêm linh kiện       [6] Import CSV\n";
    std::cout << "  [2] Tìm theo ID          [7] Export CSV\n";
    std::cout << "  [3] Cập nhật linh kiện   [8] Xem lịch sử giao dịch\n";
    std::cout << "  [4] Xoá linh kiện        [9] Thống kê B-Tree\n";
    std::cout << "  [5] Liệt kê tất cả      [10] Benchmark nhanh\n";
    std::cout << "  [0] Thoát\n";
    printDivider('-');
    std::cout << "  Tổng số linh kiện: " << db.getRecordCount() << "\n";
    printDivider('-');
}

void UserInterface::printDivider(char c, int width) const {
    std::cout << "  ";
    for (int i = 0; i < width; ++i) std::cout << c;
    std::cout << "\n";
}

int UserInterface::readIntOption(const std::string& prompt) const {
    int value;
    std::cout << "\n  " << prompt << ": ";
    while (!(std::cin >> value)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  [!] Vui lòng nhập số: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

std::string UserInterface::readStringInput(const std::string& prompt) const {
    std::cout << "  " << prompt << ": ";
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void UserInterface::pause() const {
    std::cout << "\n  [Enter để tiếp tục...]";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void UserInterface::clearScreen() const {
    std::system(CLEAR_CMD);
}

bool UserInterface::confirmAction(const std::string& msg) const {
    std::cout << "\n  " << msg << " (y/n): ";
    std::string ans;
    std::getline(std::cin, ans);
    return (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y'));
}

ComponentRecord UserInterface::buildRecordFromInput() {
    ComponentRecord rec;

    rec.id       = readIntOption("ID linh kiện (số nguyên dương)");
    rec.name     = readStringInput("Tên linh kiện");
    rec.category = readStringInput("Danh mục (CPU/RAM/GPU/SSD/...)");

    // Quantity
    rec.quantity = readIntOption("Số lượng tồn kho");

    {
        std::cout << "  Giá (VND/USD): ";
        std::string priceStr;
        std::getline(std::cin, priceStr);
        try { rec.price = std::stod(priceStr); }
        catch (...) { rec.price = 0.0; }
    }

    rec.supplier = readStringInput("Nhà cung cấp");
    return rec;
}
void UserInterface::handleAddRecord() {
    printDivider('=');
    std::cout << "  THÊM LINH KIỆN MỚI\n";
    printDivider('=');

    ComponentRecord rec = buildRecordFromInput();

    if (rec.id <= 0 || rec.name.empty()) {
        std::cout << "\n  [!] Dữ liệu không hợp lệ (ID > 0, tên không rỗng).\n";
        pause(); return;
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    bool ok = db.addRecord(rec);
    auto t1 = std::chrono::high_resolution_clock::now();
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    if (ok) {
        std::cout << "\n  ✓ Đã thêm thành công! (thời gian: "
                  << std::fixed << std::setprecision(2) << us << " µs)\n";
        std::cout << "  " << rec.toString() << "\n";
    } else {
        std::cout << "\n  [!] Thêm thất bại — ID=" << rec.id << " đã tồn tại.\n";
    }
    pause();
}

void UserInterface::handleSearchRecord() {
    printDivider('=');
    std::cout << "  TÌM KIẾM THEO ID\n";
    printDivider('=');

    int id = readIntOption("Nhập ID cần tìm");

    auto t0 = std::chrono::high_resolution_clock::now();
    const ComponentRecord* rec = db.searchRecord(id);
    auto t1 = std::chrono::high_resolution_clock::now();
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    if (rec) {
        std::cout << "\n  ✓ Tìm thấy! (thời gian: "
                  << std::fixed << std::setprecision(3) << us << " µs)\n";
        std::cout << "  " << rec->toString() << "\n";
    } else {
        std::cout << "\n  [✗] Không tìm thấy ID=" << id
                  << " (thời gian: " << us << " µs)\n";
    }
    pause();
}
void UserInterface::handleUpdateRecord() {
    printDivider('=');
    std::cout << "  CẬP NHẬT LINH KIỆN\n";
    printDivider('=');

    int id = readIntOption("Nhập ID cần cập nhật");
    const ComponentRecord* existing = db.searchRecord(id);
    if (!existing) {
        std::cout << "\n  [!] Không tìm thấy ID=" << id << ".\n";
        pause(); return;
    }

    std::cout << "  Thông tin hiện tại: " << existing->toString() << "\n";
    std::cout << "  (Nhập thông tin mới — ID không thay đổi)\n\n";

    ComponentRecord updated = buildRecordFromInput();
    updated.id = id; 

    bool ok = db.updateRecord(updated);
    if (ok) {
        std::cout << "\n  ✓ Cập nhật thành công!\n";
    } else {
        std::cout << "\n  [!] Cập nhật thất bại.\n";
    }
    pause();
}
void UserInterface::handleDeleteRecord() {
    printDivider('=');
    std::cout << "  XOÁ LINH KIỆN\n";
    printDivider('=');

    int id = readIntOption("Nhập ID cần xoá");
    const ComponentRecord* rec = db.searchRecord(id);
    if (!rec) {
        std::cout << "\n  [!] Không tìm thấy ID=" << id << ".\n";
        pause(); return;
    }

    std::cout << "  " << rec->toString() << "\n";
    if (!confirmAction("Xác nhận xoá linh kiện này?")) {
        std::cout << "  Đã huỷ.\n";
        pause(); return;
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    bool ok = db.deleteRecord(id);
    auto t1 = std::chrono::high_resolution_clock::now();
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    if (ok) {
        std::cout << "\n  ✓ Đã xoá ID=" << id
                  << " (thời gian: " << us << " µs)\n";
    } else {
        std::cout << "\n  [!] Xoá thất bại.\n";
    }
    pause();
}
void UserInterface::handleListAll() {
    printDivider('=');
    std::cout << "  DANH SÁCH LINH KIỆN (sắp xếp theo ID)\n";
    printDivider('=');

    if (db.isEmpty()) {
        std::cout << "  (Không có dữ liệu)\n";
        pause(); return;
    }
    std::cout << "  " << std::left
              << std::setw(8)  << "ID"
              << std::setw(30) << "Tên"
              << std::setw(12) << "Danh mục"
              << std::setw(8)  << "SL"
              << std::setw(14) << "Giá"
              << "Nhà cung cấp\n";
    printDivider('-');

    int count = 0;
    db.forEachSorted([&](const ComponentRecord& rec) {
        std::cout << "  " << std::left
                  << std::setw(8)  << rec.id
                  << std::setw(30) << rec.name.substr(0, 28)
                  << std::setw(12) << rec.category.substr(0, 10)
                  << std::setw(8)  << rec.quantity
                  << std::setw(14) << std::fixed << std::setprecision(2) << rec.price
                  << rec.supplier.substr(0, 20) << "\n";
        count++;
        if (count % 20 == 0) {
            printDivider('-');
            std::cout << "  (Hiển thị " << count << "/" << db.getRecordCount()
                      << " — Enter để tiếp tục, 'q' để dừng)";
            std::string s;
            std::getline(std::cin, s);
            if (!s.empty() && (s[0] == 'q' || s[0] == 'Q')) return;
            printDivider('-');
        }
    });

    printDivider('-');
    std::cout << "  Tổng: " << count << " linh kiện\n";
    pause();
}
void UserInterface::handleImportCSV() {
    printDivider('=');
    std::cout << "  IMPORT DỮ LIỆU TỪ CSV\n";
    printDivider('=');

    std::string filename = readStringInput("Nhập tên file CSV (vd: data.csv)");
    if (filename.empty()) { pause(); return; }

    auto t0 = std::chrono::high_resolution_clock::now();
    FileResult result = FileHandler::importCSV(db, filename);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "\n  " << (result.success ? "✓" : "✗") << " "
              << result.message << "\n";
    std::cout << "  Thời gian: " << std::fixed << std::setprecision(2) << ms << " ms\n";
    pause();
}
void UserInterface::handleExportCSV() {
    printDivider('=');
    std::cout << "  EXPORT DỮ LIỆU RA CSV\n";
    printDivider('=');

    std::string filename = readStringInput("Nhập tên file CSV (vd: export.csv)");
    if (filename.empty()) { pause(); return; }

    auto t0 = std::chrono::high_resolution_clock::now();
    FileResult result = FileHandler::exportCSV(db, filename);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "\n  " << (result.success ? "✓" : "✗") << " "
              << result.message << "\n";
    std::cout << "  Thời gian: " << std::fixed << std::setprecision(2) << ms << " ms\n";
    pause();
}
void UserInterface::handleViewHistory() {
    printDivider('=');
    std::cout << "  LỊCH SỬ GIAO DỊCH (DSLK đơn)\n";
    printDivider('=');

    int limit = readIntOption("Hiển thị bao nhiêu giao dịch gần nhất? (mặc định 20)");
    if (limit <= 0) limit = 20;
    db.printHistory(limit);
    pause();
}
void UserInterface::handleStatistics() {
    printDivider('=');
    std::cout << "  THỐNG KÊ HỆ THỐNG\n";
    printDivider('=');

    std::cout << "\n  ┌── B-Tree Index ──────────────────────────────┐\n";
    std::cout << "  │ Bậc t (degree)     : " << std::setw(6) << BTREE_T << "\n";
    std::cout << "  │ Số record          : " << std::setw(6) << db.getRecordCount() << "\n";
    std::cout << "  │ Chiều cao cây      : " << std::setw(6) << db.getBTreeHeight() << "\n";
    std::cout << "  │ Số node            : " << std::setw(6) << db.getBTreeNodeCount() << "\n";
    std::cout << "  │ Max keys/node      : " << std::setw(6) << MAX_KEYS << " (2t-1)\n";
    std::cout << "  │ Phức tạp tìm kiếm  : O(log_" << BTREE_T << "(n))\n";
    std::cout << "  └─────────────────────────────────────────────┘\n";

    std::cout << "\n  ┌── Transaction History (DSLK) ────────────────┐\n";
    std::cout << "  │ Số giao dịch đã ghi: "
              << std::setw(6) << db.getHistory().getSize() << "\n";
    std::cout << "  │ Giới hạn tối đa    : "
              << std::setw(6) << MAX_HISTORY << "\n";
    std::cout << "  └─────────────────────────────────────────────┘\n";

    pause();
}
void UserInterface::handleQuickBenchmark() {
    printDivider('=');
    std::cout << "  BENCHMARK NHANH (10,000 records)\n";
    printDivider('=');

    const int N = 10000;
    ComponentDatabase testDb;
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 1; i <= N; ++i) {
        ComponentRecord rec(i, "TestComp_" + std::to_string(i), "CPU",
                            100, i * 1.5, "TestSupplier");
        testDb.addRecord(rec);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double insertMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    t0 = std::chrono::high_resolution_clock::now();
    int found = 0;
    for (int i = 1; i <= N; ++i) {
        if (testDb.searchRecord(i)) found++;
    }
    t1 = std::chrono::high_resolution_clock::now();
    double searchMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 1; i <= N / 2; ++i) {
        testDb.deleteRecord(i);
    }
    t1 = std::chrono::high_resolution_clock::now();
    double deleteMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "\n  ┌── Kết quả (" << N << " records) ─────────────────┐\n";
    std::cout << "  │ INSERT  " << N << " records: "
              << std::fixed << std::setprecision(2) << std::setw(10) << insertMs << " ms"
              << " (" << (insertMs * 1000.0 / N) << " µs/op)\n";
    std::cout << "  │ SEARCH  " << N << " records: "
              << std::setw(10) << searchMs << " ms"
              << " (" << (searchMs * 1000.0 / N) << " µs/op)\n";
    std::cout << "  │ DELETE  " << N/2 << " records: "
              << std::setw(10) << deleteMs << " ms"
              << " (" << (deleteMs * 1000.0 / (N/2)) << " µs/op)\n";
    std::cout << "  │ B-Tree height: " << testDb.getBTreeHeight() << "\n";
    std::cout << "  │ Kết quả tìm  : " << found << "/" << N << "\n";
    std::cout << "  └─────────────────────────────────────────────┘\n";

    std::cout << "  (testDb tạm thời — không ảnh hưởng CSDL chính)\n";
    pause();
}