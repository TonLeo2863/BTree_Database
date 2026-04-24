#pragma once
#include <string>
#include "../2_Database/Record.h"
#include "../3_FileIO/FileHandler.h"

class UserInterface {
public:
    explicit UserInterface(AccountDatabase& db);
    void run();

private:
    AccountDatabase& db;
    void   printBanner()       const;
    void   printMenu()         const;
    void   printDivider(char c = '-', int width = 60) const;
    int    readIntOption(const std::string& prompt) const;
    double readDoubleInput(const std::string& prompt) const;
    std::string readStringInput(const std::string& prompt) const;
    void   pause()             const;
    void   clearScreen()       const;

    void handleAddRecord();
    void handleSearchRecord();
    void handleUpdateRecord();
    void handleDeleteRecord();
    void handleListAll();
    void handleRangeScan(); // MỚI: Truy vấn theo khoảng
    void handleImportCSV();
    void handleExportCSV();
    void handleViewHistory();
    void handleStatistics();
    void handleQuickBenchmark();

    AccountRecord buildRecordFromInput();
    bool confirmAction(const std::string& msg) const;
};