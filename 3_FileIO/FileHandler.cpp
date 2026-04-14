#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdio>     
#include <iomanip>
#include <sys/stat.h> 
const std::string FileHandler::CSV_HEADER =
    "id,name,category,quantity,price,supplier";

FileResult FileHandler::importCSV(ComponentDatabase& db,
                                  const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return FileResult(false, 0, "Cannot open file: " + filename);
    }

    std::string line;
    int inserted  = 0;
    int skipped   = 0;
    int lineNum   = 0;

    while (std::getline(file, line)) {
        lineNum++;

        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (lineNum == 1 && line.rfind("id", 0) == 0) continue;

        bool ok = false;
        ComponentRecord rec = parseCSVRow(line, ok);
        if (!ok) {
            skipped++;
            continue;
        }

        if (db.addRecord(rec)) {
            inserted++;
        } else {
            skipped++;
        }
    }

    file.close();

    std::ostringstream msg;
    msg << "Imported " << inserted << " records"
        << " (skipped " << skipped << " invalid/duplicate rows)"
        << " from [" << filename << "]";

    return FileResult(true, inserted, msg.str());
}
FileResult FileHandler::exportCSV(const ComponentDatabase& db,
                                  const std::string& filename) {
    if (fileExists(filename)) {
        if (!createBackup(filename)) {
            std::cerr << "[FileHandler] Warning: backup failed for " << filename << "\n";
        }
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        return FileResult(false, 0, "Cannot write to file: " + filename);
    }

    file << CSV_HEADER << "\n";

    int written = 0;
    db.forEachSorted([&](const ComponentRecord& rec) {
        file << rec.toCSV() << "\n";
        written++;
    });

    file.flush();
    file.close();

    std::ostringstream msg;
    msg << "Exported " << written << " records to [" << filename << "]"
        << " (backup: " << getBackupFilename(filename) << ")";

    return FileResult(true, written, msg.str());
}
bool FileHandler::createBackup(const std::string& filename) {
    if (!fileExists(filename)) return false;

    std::ifstream src(filename, std::ios::binary);
    if (!src.is_open()) return false;

    std::string bakName = getBackupFilename(filename);
    std::ofstream dst(bakName, std::ios::binary | std::ios::trunc);
    if (!dst.is_open()) return false;

    dst << src.rdbuf();

    bool ok = src.good() && dst.good();
    src.close();
    dst.close();
    return ok;
}
bool FileHandler::restoreBackup(const std::string& filename) {
    std::string bakName = getBackupFilename(filename);
    if (!fileExists(bakName)) return false;

    std::ifstream src(bakName, std::ios::binary);
    if (!src.is_open()) return false;

    std::ofstream dst(filename, std::ios::binary | std::ios::trunc);
    if (!dst.is_open()) return false;

    dst << src.rdbuf();

    bool ok = src.good() && dst.good();
    src.close();
    dst.close();
    return ok;
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

std::string FileHandler::getBackupFilename(const std::string& filename) {
    return filename + ".bak";
}
FileResult FileHandler::generateSampleCSV(const std::string& filename, int n) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return FileResult(false, 0, "Cannot create file: " + filename);
    }
    const char* categories[] = {"CPU","RAM","GPU","SSD","HDD",
                                  "Motherboard","PSU","Cooling","Case","Monitor"};
    const char* suppliers[]  = {"Intel","AMD","Samsung","WD","Seagate",
                                  "ASUS","MSI","Gigabyte","Corsair","EVGA"};
    const int catCount = 10;
    const int supCount = 10;

    file << CSV_HEADER << "\n";

    for (int i = 1; i <= n; ++i) {
        int    catIdx = (i - 1) % catCount;
        int    supIdx = (i - 1) % supCount;
        int    qty    = 10 + (i * 7) % 490;     
        double price  = 100.0 + (i * 13.7);     
        std::ostringstream name;
        name << categories[catIdx] << "_" << std::setw(6) << std::setfill('0') << i;

        file << i << ","
             << name.str() << ","
             << categories[catIdx] << ","
             << qty << ","
             << std::fixed << std::setprecision(2) << price << ","
             << suppliers[supIdx] << "\n";
    }

    file.close();

    std::ostringstream msg;
    msg << "Generated " << n << " sample records in [" << filename << "]";
    return FileResult(true, n, msg.str());
}

std::vector<std::string> FileHandler::splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i; 
                } else {
                    inQuotes = false; 
                }
            } else {
                field += c;
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                fields.push_back(field);
                field.clear();
            } else {
                field += c;
            }
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

ComponentRecord FileHandler::parseCSVRow(const std::string& line, bool& ok) {
    ok = false;
    ComponentRecord rec;

    std::vector<std::string> fields = splitCSV(line);
    if (fields.size() < 5) return rec;

    try {
        rec.id       = std::stoi(trimWhitespace(fields[0]));
        rec.name     = trimWhitespace(fields[1]);
        rec.category = trimWhitespace(fields[2]);
        rec.quantity = std::stoi(trimWhitespace(fields[3]));
        rec.price    = std::stod(trimWhitespace(fields[4]));
        rec.supplier = (fields.size() >= 6) ? trimWhitespace(fields[5]) : "";

        if (rec.id <= 0) return rec; 
        ok = true;
    } catch (...) {
        ok = false; 
    }

    return rec;
}