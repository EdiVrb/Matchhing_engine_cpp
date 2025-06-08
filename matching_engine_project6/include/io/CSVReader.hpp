// ===== include/io/CSVReader.hpp =====
#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <functional>

class CSVReader {
private:
    std::ifstream file_;
    char delimiter_;
    size_t lineNumber_;
    
public:
    explicit CSVReader(const std::string& filename, char delim = ',');
    ~CSVReader();
    
    void readLine(std::function<void(const std::vector<std::string>&)> callback);
    bool hasNext() const;
    
private:
    std::vector<std::string> parseLine(const std::string& line);
};
