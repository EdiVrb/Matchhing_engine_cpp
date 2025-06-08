#pragma once
#include "types/OrderTypes.hpp"
#include "types/Enums.hpp"
#include <memory>
#include <string>

class Order {
private:
    Timestamp timestamp_;
    OrderId orderId_;
    std::string instrument_;
    Side side_;
    OrderType type_;
    Quantity quantity_;
    Quantity remainingQuantity_;
    Quantity executedQuantity_;
    Price price_;
    Price executionPrice_;
    OrderStatus status_;
    OrderId counterpartyId_;

public:
    Order(Timestamp ts, OrderId id, std::string instrument, 
          Side side, OrderType type, Quantity qty, Price price);
    
    // Getters inline pour performance
    inline Timestamp getTimestamp() const { return timestamp_; }
    inline OrderId getOrderId() const { return orderId_; }
    inline const std::string& getInstrument() const { return instrument_; }
    inline Side getSide() const { return side_; }
    inline OrderType getType() const { return type_; }
    inline Quantity getQuantity() const { return quantity_; }
    inline Quantity getRemainingQuantity() const { return remainingQuantity_; }
    inline Quantity getExecutedQuantity() const { return executedQuantity_; }
    inline Price getPrice() const { return price_; }
    inline Price getExecutionPrice() const { return executionPrice_; }
    inline OrderStatus getStatus() const { return status_; }
    inline OrderId getCounterpartyId() const { return counterpartyId_; }
    
    void updateQuantity(Quantity newQty);
    void updatePrice(Price newPrice);
    void execute(Quantity executedQty, Price execPrice, OrderId counterparty);
    void cancel();
    
    bool isActive() const { 
        return status_ == OrderStatus::PENDING || status_ == OrderStatus::PARTIALLY_EXECUTED; 
    }
};

using OrderPtr = std::shared_ptr<Order>;