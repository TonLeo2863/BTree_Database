#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdio>    
#include <iomanip>
#include <sys/stat.h>

const std::string FileHandler::CSV_HEADER = "stk,chu_the,so_du,chi_nhanh";

FileResult FileHandler::importCSV(AccountDatabase& db, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return FileResult(false, 0, "Không thể mở file: " + filename);

    std::string line;
    int inserted = 0, skipped = 0, lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (lineNum == 1 && line.rfind("stk", 0) == 0) continue;

        bool ok = false;
        AccountRecord rec = parseCSVRow(line, ok);
        if (!ok) { skipped++; continue; }
        if (db.addRecord(rec)) inserted++;
        else skipped++;
    }
    file.close();
    std::ostringstream msg;
    msg << "Đã import " << inserted << " tài khoản (bỏ qua " << skipped << " lỗi/trùng) từ [" << filename << "]";
    return FileResult(true, inserted, msg.str());
}

FileResult FileHandler::exportCSV(const AccountDatabase& db, const std::string& filename) {
    if (fileExists(filename) && !createBackup(filename)) {
        std::cerr << "[FileHandler] Cảnh báo: Lỗi backup cho " << filename << "\n";
    }

    std::ofstream file(filename);
    if (!file.is_open()) return FileResult(false, 0, "Không thể ghi file: " + filename);

    file << CSV_HEADER << "\n";
    int written = 0;
    db.forEachSorted([&](const AccountRecord& rec) {
        file << rec.toCSV() << "\n";
        written++;
    });

    file.flush();
    file.close();
    std::ostringstream msg;
    msg << "Đã export " << written << " tài khoản ra [" << filename << "]";
    return FileResult(true, written, msg.str());
}

bool FileHandler::createBackup(const std::string& filename) {
    if (!fileExists(filename)) return false;
    std::ifstream src(filename, std::ios::binary);
    std::string bakName = getBackupFilename(filename);
    std::ofstream dst(bakName, std::ios::binary | std::ios::trunc);
    dst << src.rdbuf();
    return src.good() && dst.good();
}

bool FileHandler::restoreBackup(const std::string& filename) {
    std::string bakName = getBackupFilename(filename);
    if (!fileExists(bakName)) return false;
    std::ifstream src(bakName, std::ios::binary);
    std::ofstream dst(filename, std::ios::binary | std::ios::trunc);
    dst << src.rdbuf();
    return src.good() && dst.good();
}

bool FileHandler::fileExists(const std::string& filename) {
    std::ifstream f(filename);
    return f.good();
}

long FileHandler::getFileSize(const std::string& filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) == 0) return static_cast<long>(st.st_size);
    return -1L;
}

std::string FileHandler::getBackupFilename(const std::string& filename) { return filename + ".bak"; }

FileResult FileHandler::generateSampleCSV(const std::string& filename, int n) {
    std::ofstream file(filename);
    if (!file.is_open()) return FileResult(false, 0, "Không thể tạo file: " + filename);

    const char* chiNhanh[] = {"Tan Binh", "Q1", "Q3", "Phu Nhuan", "Go Vap", "Thu Duc", "Binh Thanh"};
    int cnCount = 7;

    file << CSV_HEADER << "\n";
    for (int i = 1; i <= n; ++i) {
        double soDu = 1000000.0 + (i * 150000.0);
        std::ostringstream name;
        name << "Khach Hang " << i;
        file << i << "," << name.str() << "," << std::fixed << std::setprecision(0) << soDu << "," << chiNhanh[i % cnCount] << "\n";
    }
    file.close();
    return FileResult(true, n, "Đã tạo " + std::to_string(n) + " records mẫu.");
}

std::vector<std::string> FileHandler::splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { field += '"'; ++i; } 
                else inQuotes = false;
            } else field += c;
        } else {
            if (c == '"') inQuotes = true;
            else if (c == ',') { fields.push_back(field); field.clear(); } 
            else field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

std::string FileHandler::trimWhitespace(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

AccountRecord FileHandler::parseCSVRow(const std::string& line, bool& ok) {
    ok = false;
    AccountRecord rec;
    std::vector<std::string> fields = splitCSV(line);
    if (fields.size() < 4) return rec;

    try {
        rec.stk      = std::stoi(trimWhitespace(fields[0]));
        rec.chuThe   = trimWhitespace(fields[1]);
        rec.soDu     = std::stod(trimWhitespace(fields[2]));
        rec.chiNhanh = trimWhitespace(fields[3]);
        if (rec.stk <= 0) return rec;
        ok = true;
    } catch (...) { ok = false; }
    return rec;
}