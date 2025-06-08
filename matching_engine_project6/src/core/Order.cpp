// ===== src/core/Order.cpp =====
#include "core/Order.hpp"
#include "exceptions/Exceptions.hpp"

Order::Order(Timestamp ts, OrderId id, std::string instrument, 
             Side side, OrderType type, Quantity qty, Price price)
    : timestamp_(ts), orderId_(id), instrument_(std::move(instrument)),
      side_(side), type_(type), quantity_(qty), remainingQuantity_(qty),
      executedQuantity_(0), price_(price), executionPrice_(0.0),
      status_(OrderStatus::PENDING), counterpartyId_(0) {
    
    if (id == 0) {
        throw InvalidOrderException(id, "Order ID cannot be zero");
    }
    if (instrument_.empty()) {
        throw InvalidOrderException(id, "Instrument cannot be empty");
    }
    if (qty == 0) {
        throw InvalidOrderException(id, "Quantity must be positive");
    }
    
    // IMPORTANT: Forcer le prix à 0 pour les ordres MARKET
    if (type == OrderType::MARKET) {
        price_ = 0.0;
    } else if (price <= 0.0) {
        // Pour les ordres LIMIT, le prix doit être positif
        throw InvalidOrderException(id, "Limit order must have positive price");
    }
}

void Order::updateQuantity(Quantity newQty) {
    if (newQty == 0) {
        throw InvalidOrderException(orderId_, "Cannot update to zero quantity");
    }
    quantity_ = newQty;
    remainingQuantity_ = newQty - executedQuantity_;
}

void Order::updatePrice(Price newPrice) {
    // Les ordres MARKET ont toujours un prix de 0
    if (type_ == OrderType::MARKET) {
        price_ = 0.0;
        return;
    }
    
    if (type_ == OrderType::LIMIT && newPrice <= 0.0) {
        throw InvalidOrderException(orderId_, "Invalid price for limit order");
    }
    price_ = newPrice;
}

void Order::execute(Quantity executedQty, Price execPrice, OrderId counterparty) {
    if (executedQty > remainingQuantity_) {
        throw InvalidOrderException(orderId_, "Execution quantity exceeds remaining");
    }
    
    executedQuantity_ += executedQty;
    remainingQuantity_ -= executedQty;
    executionPrice_ = execPrice;
    counterpartyId_ = counterparty;
    
    if (remainingQuantity_ == 0) {
        status_ = OrderStatus::EXECUTED;
    } else {
        status_ = OrderStatus::PARTIALLY_EXECUTED;
    }
}

void Order::cancel() {
    if (status_ == OrderStatus::EXECUTED) {
        throw InvalidOrderException(orderId_, "Cannot cancel executed order");
    }
    status_ = OrderStatus::CANCELED;
}