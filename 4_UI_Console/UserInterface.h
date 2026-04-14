#pragma once
#include <string>
#include "../2_Database/Record.h"
#include "../3_FileIO/FileHandler.h"

class UserInterface {
public:
    explicit UserInterface(ComponentDatabase& db);
    ~UserInterface() = default;
    void run();

private:
    ComponentDatabase& db;
    void   printBanner()       const;
    void   printMenu()         const;
    void   printDivider(char c = '-', int width = 58) const;
    int    readIntOption(const std::string& prompt) const;
    std::string readStringInput(const std::string& prompt) const;
    void   pause()             const;
    void   clearScreen()       const;
    void handleAddRecord();
    void handleSearchRecord();
    void handleUpdateRecord();
    void handleDeleteRecord();
    void handleListAll();
    void handleImportCSV();
    void handleExportCSV();
    void handleViewHistory();
    void handleStatistics();
    void handleQuickBenchmark();
    ComponentRecord buildRecordFromInput();
    bool confirmAction(const std::string& msg) const;
};