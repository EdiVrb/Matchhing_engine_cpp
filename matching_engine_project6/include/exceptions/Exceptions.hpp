#pragma once
#include <exception>
#include <string>
#include <sstream>
#include "types/OrderTypes.hpp"

class MatchingEngineException : public std::exception {
protected:
    std::string message_;
    
public:
    explicit MatchingEngineException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override { return message_.c_str(); }
};

class InvalidOrderException : public MatchingEngineException {
public:
    InvalidOrderException(OrderId orderId, const std::string& reason)
        : MatchingEngineException(buildMessage(orderId, reason)) {}
    
private:
    static std::string buildMessage(OrderId orderId, const std::string& reason) {
        std::ostringstream oss;
        oss << "Invalid order " << orderId << ": " << reason;
        return oss.str();
    }
};

class OrderNotFoundException : public MatchingEngineException {
public:
    explicit OrderNotFoundException(OrderId orderId)
        : MatchingEngineException("Order " + std::to_string(orderId) + " not found") {}
};

class CSVParsingException : public MatchingEngineException {
public:
    CSVParsingException(size_t lineNumber, const std::string& reason)
        : MatchingEngineException("Line " + std::to_string(lineNumber) + ": " + reason) {}
};

class FileIOException : public MatchingEngineException {
public:
    FileIOException(const std::string& filename, const std::string& operation)
        : MatchingEngineException("File " + operation + " error: " + filename) {}
};