
// ===== include/io/CSVWriter.hpp =====
#pragma once
#include "core/OrderEvent.hpp"
#include <fstream>
#include <string>

class CSVWriter {
private:
    std::ofstream file_;
    
public:
    explicit CSVWriter(const std::string& filename);
    ~CSVWriter();
    
    void writeHeader();
    void writeEvent(const OrderEvent& event);
};
