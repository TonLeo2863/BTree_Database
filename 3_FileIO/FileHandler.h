#pragma once
#include <string>
#include <vector>
#include "../2_Database/Record.h"

struct FileResult {
    bool        success;
    int         recordsProcessed;
    std::string message;

    FileResult(bool ok, int n, const std::string& msg)
        : success(ok), recordsProcessed(n), message(msg) {}
};

class FileHandler {
public:
    static const std::string CSV_HEADER; 
    static FileResult importCSV(ComponentDatabase& db,
                                const std::string& filename);
    static FileResult exportCSV(const ComponentDatabase& db,
                                const std::string& filename);

    static bool createBackup(const std::string& filename);

    static bool restoreBackup(const std::string& filename);

    static bool fileExists   (const std::string& filename);
    static long getFileSize  (const std::string& filename);
    static std::string getBackupFilename(const std::string& filename);
    static FileResult generateSampleCSV(const std::string& filename, int n);

private:
    static ComponentRecord  parseCSVRow   (const std::string& line, bool& ok);
    static std::vector<std::string> splitCSV(const std::string& line);
    static std::string trimWhitespace(const std::string& s);
    FileHandler() = delete;
};