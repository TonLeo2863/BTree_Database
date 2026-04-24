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

UserInterface::UserInterface(AccountDatabase& db) : db(db) {}

void UserInterface::run() {
    printBanner();
    bool running = true;
    while (running) {
        printMenu();
        int choice = readIntOption("Chọn chức năng");

        switch (choice) {
            case 1:  handleAddRecord();      break;
            case 2:  handleSearchRecord();   break;
            case 3:  handleUpdateRecord();   break;
            case 4:  handleDeleteRecord();   break;
            case 5:  handleListAll();        break;
            case 6:  handleRangeScan();      break; // Truy vấn theo khoảng
            case 7:  handleImportCSV();      break;
            case 8:  handleExportCSV();      break;
            case 9:  handleViewHistory();    break;
            case 10: handleStatistics();     break;
            case 11: handleQuickBenchmark(); break;
            case 0:
                if (confirmAction("Bạn có chắc muốn thoát?")) {
                    std::cout << "\n  Tạm biệt! Hệ thống đã đóng.\n\n";
                    running = false;
                }
                break;
            default:
                std::cout << "\n  [!] Lựa chọn không hợp lệ.\n";
                pause();
        }
    }
}

void UserInterface::printBanner() const {
    clearScreen();
    std::cout << "\n";
    std::cout << "  ╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║     HỆ THỐNG QUẢN LÝ TÀI KHOẢN NGÂN HÀNG (B-TREE INDEX)    ║\n";
    std::cout << "  ║      Đề tài 23: Cấu trúc dữ liệu và Giải thuật (C++)       ║\n";
    std::cout << "  ╚════════════════════════════════════════════════════════════╝\n\n";
}

void UserInterface::printMenu() const {
    std::cout << "\n";
    printDivider('-');
    std::cout << "  [1] Mở tài khoản (Insert)    [7] Import dữ liệu CSV\n";
    std::cout << "  [2] Tìm theo STK (Search)    [8] Export dữ liệu CSV\n";
    std::cout << "  [3] Nạp/Chuyển tiền (Update) [9] Xem lịch sử giao dịch (DSLK)\n";
    std::cout << "  [4] Đóng tài khoản (Delete) [10] Thống kê Khách Hàng VIP\n";
    std::cout << "  [5] Liệt kê Inorder         [11] Đua chiều cao: B-Tree vs BST\n";
    std::cout << "  [6] Truy vấn khoảng (X->Y)\n";
    std::cout << "  [0] Thoát\n";
    printDivider('-');
    std::cout << "  Tổng số tài khoản: " << db.getRecordCount() << "\n";
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

double UserInterface::readDoubleInput(const std::string& prompt) const {
    double value;
    std::cout << "  " << prompt << ": ";
    while (!(std::cin >> value)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  [!] Vui lòng nhập số hợp lệ: ";
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

void UserInterface::clearScreen() const { std::system(CLEAR_CMD); }

bool UserInterface::confirmAction(const std::string& msg) const {
    std::cout << "\n  " << msg << " (y/n): ";
    std::string ans;
    std::getline(std::cin, ans);
    return (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y'));
}

AccountRecord UserInterface::buildRecordFromInput() {
    AccountRecord rec;
    rec.stk      = readIntOption("Số tài khoản (STK)");
    rec.chuThe   = readStringInput("Tên chủ thẻ");
    rec.soDu     = readDoubleInput("Số dư ban đầu (VNĐ)");
    rec.chiNhanh = readStringInput("Chi nhánh (vd: Tan Binh)");
    return rec;
}

void UserInterface::handleAddRecord() {
    printDivider('=');
    std::cout << "  MỞ TÀI KHOẢN MỚI\n";
    printDivider('=');

    AccountRecord rec = buildRecordFromInput();
    if (rec.stk <= 0 || rec.chuThe.empty()) {
        std::cout << "\n  [!] Dữ liệu không hợp lệ.\n";
        pause(); return;
    }

    if (db.addRecord(rec)) {
        std::cout << "\n  ✓ Đã mở tài khoản thành công!\n";
        std::cout << "  " << rec.toString() << "\n";
    } else {
        std::cout << "\n  [!] Thất bại — STK=" << rec.stk << " đã tồn tại.\n";
    }
    pause();
}

void UserInterface::handleSearchRecord() {
    printDivider('=');
    std::cout << "  TÌM KIẾM TÀI KHOẢN (INDEX SEEK)\n";
    printDivider('=');

    int stk = readIntOption("Nhập STK cần tìm");
    auto t0 = std::chrono::high_resolution_clock::now();
    const AccountRecord* rec = db.searchRecord(stk);
    auto t1 = std::chrono::high_resolution_clock::now();
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    if (rec) {
        std::cout << "\n  ✓ Tìm thấy! (Thời gian tìm trên B-Tree: " << us << " µs)\n";
        std::cout << "  " << rec->toString() << "\n";
    } else {
        std::cout << "\n  [✗] Không tìm thấy STK=" << stk << "\n";
    }
    pause();
}

void UserInterface::handleUpdateRecord() {
    printDivider('=');
    std::cout << "  NẠP / CHUYỂN TIỀN\n";
    printDivider('=');

    int stk = readIntOption("Nhập STK cần giao dịch");
    const AccountRecord* existing = db.searchRecord(stk);
    if (!existing) {
        std::cout << "\n  [!] Không tìm thấy STK=" << stk << ".\n";
        pause(); return;
    }

    std::cout << "  Thông tin hiện tại: " << existing->toString() << "\n";
    double tien = readDoubleInput("Nhập số tiền nạp (+ dương) hoặc rút/chuyển (- âm)");
    
    AccountRecord updated = *existing;
    updated.soDu += tien;

    if (db.updateRecord(updated)) {
        std::cout << "\n  ✓ Giao dịch thành công. Số dư mới: " << std::fixed << std::setprecision(0) << updated.soDu << "\n";
    } else {
        std::cout << "\n  [!] Giao dịch thất bại.\n";
    }
    pause();
}

void UserInterface::handleDeleteRecord() {
    printDivider('=');
    std::cout << "  ĐÓNG TÀI KHOẢN\n";
    printDivider('=');

    int stk = readIntOption("Nhập STK cần đóng");
    const AccountRecord* rec = db.searchRecord(stk);
    if (!rec) {
        std::cout << "\n  [!] Không tìm thấy STK=" << stk << ".\n";
        pause(); return;
    }

    std::cout << "  " << rec->toString() << "\n";
    if (!confirmAction("Xác nhận đóng tài khoản này và xóa lịch sử?")) {
        std::cout << "  Đã huỷ.\n";
        pause(); return;
    }

    if (db.deleteRecord(stk)) std::cout << "\n  ✓ Đã xoá STK=" << stk << "\n";
    else std::cout << "\n  [!] Xoá thất bại.\n";
    pause();
}

void UserInterface::handleListAll() {
    printDivider('=');
    std::cout << "  DANH SÁCH TÀI KHOẢN (DUYỆT INORDER B-TREE)\n";
    printDivider('=');

    if (db.isEmpty()) { std::cout << "  (Không có dữ liệu)\n"; pause(); return; }
    int count = 0;
    db.forEachSorted([&](const AccountRecord& rec) {
        std::cout << "  " << rec.toString() << "\n";
        count++;
    });
    printDivider('-');
    std::cout << "  Tổng: " << count << " tài khoản\n";
    pause();
}

void UserInterface::handleRangeScan() {
    printDivider('=');
    std::cout << "  TRUY VẤN THEO KHOẢNG (RANGE SCAN)\n";
    printDivider('=');
    
    int minX = readIntOption("Nhập STK Bắt đầu (X)");
    int maxY = readIntOption("Nhập STK Kết thúc (Y)");

    int count = 0;
    auto t0 = std::chrono::high_resolution_clock::now();
    std::cout << "\n  KẾT QUẢ TÌM KIẾM:\n";
    db.rangeScan(minX, maxY, [&](const AccountRecord& rec) {
        std::cout << "  " << rec.toString() << "\n";
        count++;
    });
    auto t1 = std::chrono::high_resolution_clock::now();
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    printDivider('-');
    std::cout << "  Đã tìm thấy " << count << " tài khoản trong khoảng [" << minX << " -> " << maxY << "].\n";
    std::cout << "  Thời gian thực thi: " << us << " µs.\n";
    pause();
}

void UserInterface::handleImportCSV() {
    std::string filename = readStringInput("Nhập tên file CSV");
    FileResult result = FileHandler::importCSV(db, filename);
    std::cout << "\n  " << result.message << "\n";
    pause();
}

void UserInterface::handleExportCSV() {
    std::string filename = readStringInput("Nhập tên file CSV");
    FileResult result = FileHandler::exportCSV(db, filename);
    std::cout << "\n  " << result.message << "\n";
    pause();
}

void UserInterface::handleViewHistory() { db.printHistory(20); pause(); }

void UserInterface::handleStatistics() {
    printDivider('=');
    std::cout << "  THỐNG KÊ HỆ THỐNG & TÌM VIP\n";
    printDivider('=');

    std::cout << "  Bậc B-Tree (M)   : " << (MAX_KEYS + 1) << "\n";
    std::cout << "  Số tài khoản     : " << db.getRecordCount() << "\n";
    std::cout << "  Chiều cao B-Tree : " << db.getBTreeHeight() << "\n";

    double maxSoDu = -1;
    std::string vipName = "";
    db.forEachSorted([&](const AccountRecord& rec) {
        if (rec.soDu > maxSoDu) { maxSoDu = rec.soDu; vipName = rec.chuThe; }
    });

    std::cout << "\n  🏆 KHÁCH HÀNG VIP (SỐ DƯ LỚN NHẤT):\n";
    if (vipName != "") {
        std::cout << "  Chủ thẻ: " << vipName << " | Số dư: " << std::fixed << std::setprecision(0) << maxSoDu << " VNĐ\n";
    } else {
        std::cout << "  (Chưa có dữ liệu)\n";
    }
    pause();
}

void UserInterface::handleQuickBenchmark() {
    std::cout << "\n  Chuyển sang chức năng Benchmark (100k dữ liệu) so sánh với BST.\n";
    std::cout << "  Vui lòng chạy lại chương trình với cờ '--benchmark' để xem kết quả.\n";
    pause();
}