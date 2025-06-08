
// ===== src/io/CSVReader.cpp =====
#include "io/CSVReader.hpp"
#include "exceptions/Exceptions.hpp"
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>

CSVReader::CSVReader(const std::string& filename, char delim) 
    : delimiter_(delim), lineNumber_(0) {
    file_.open(filename);
    if (!file_.is_open()) {
        throw FileIOException(filename, "open");
    }
    
    // Skip header
    std::string header;
    if (std::getline(file_, header)) {
        lineNumber_++;
    }
}

CSVReader::~CSVReader() {
    if (file_.is_open()) {
        file_.close();
    }
}

void CSVReader::readLine(std::function<void(const std::vector<std::string>&)> callback) {
    std::string line;
    
    while (std::getline(file_, line)) {
        lineNumber_++;
        
        if (line.empty()) continue;
        
        try {
            auto fields = parseLine(line);
            callback(fields);
        } catch (const std::exception& e) {
            throw CSVParsingException(lineNumber_, e.what());
        }
    }
}

bool CSVReader::hasNext() const {
    return file_.good() && !file_.eof();
}

std::vector<std::string> CSVReader::parseLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    
    while (std::getline(ss, field, delimiter_)) {
        // Trim whitespace
        size_t start = field.find_first_not_of(" \t");
        size_t end = field.find_last_not_of(" \t");
        
        if (start != std::string::npos && end != std::string::npos) {
            fields.push_back(field.substr(start, end - start + 1));
        } else {
            fields.push_back("");
        }
    }
    
    return fields;
}