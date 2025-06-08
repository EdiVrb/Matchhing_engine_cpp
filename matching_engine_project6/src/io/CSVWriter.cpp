// ===== src/io/CSVWriter.cpp =====
#include "io/CSVWriter.hpp"
#include "exceptions/Exceptions.hpp"
#include <iomanip>

CSVWriter::CSVWriter(const std::string& filename) {
    file_.open(filename);
    if (!file_.is_open()) {
        throw FileIOException(filename, "open for writing");
    }
    file_ << std::fixed << std::setprecision(2);
}

CSVWriter::~CSVWriter() {
    if (file_.is_open()) {
        file_.close();
    }
}

void CSVWriter::writeHeader() {
    file_ << "timestamp,order_id,instrument,side,type,quantity,price,action,"
          << "status,executed_quantity,execution_price,counterparty_id\n";
}

void CSVWriter::writeEvent(const OrderEvent& event) {
    // Pour les ordres MARKET, toujours afficher 0 dans la colonne price
    Price displayPrice = event.price;
    if (event.type == OrderType::MARKET) {
        displayPrice = 0.0;
    }
    
    file_ << event.actionTimestamp << ","
          << event.orderId << ","
          << event.instrument << ","
          << (event.side == Side::BUY ? "BUY" : "SELL") << ","
          << (event.type == OrderType::LIMIT ? "LIMIT" : "MARKET") << ","
          << event.displayQuantity << ","
          << displayPrice << ","  // Utiliser displayPrice au lieu de event.price
          << (event.action == Action::NEW ? "NEW" : 
              event.action == Action::MODIFY ? "MODIFY" : "CANCEL") << ","
          << (event.status == OrderStatus::PENDING ? "PENDING" :
              event.status == OrderStatus::EXECUTED ? "EXECUTED" :
              event.status == OrderStatus::PARTIALLY_EXECUTED ? "PARTIALLY_EXECUTED" :
              event.status == OrderStatus::CANCELED ? "CANCELED" : "REJECTED") << ","
          << event.executedQuantity << ","
          << event.executionPrice << ","
          << event.counterpartyId << "\n";
}