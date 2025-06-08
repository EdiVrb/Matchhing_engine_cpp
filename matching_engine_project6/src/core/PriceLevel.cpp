

// ===== src/core/PriceLevel.cpp =====
#include "core/PriceLevel.hpp"
#include "exceptions/Exceptions.hpp"

PriceLevel::PriceLevel(Price price) 
    : price_(price), totalQuantity_(0) {}

void PriceLevel::addOrder(OrderPtr order) {
    orders_.push_back(order);
    totalQuantity_ += order->getRemainingQuantity();
}

void PriceLevel::removeOrder(OrderId orderId) {
    auto it = std::find_if(orders_.begin(), orders_.end(),
        [orderId](const OrderPtr& order) { 
            return order->getOrderId() == orderId; 
        });
    
    if (it == orders_.end()) {
        throw OrderNotFoundException(orderId);
    }
    
    totalQuantity_ -= (*it)->getRemainingQuantity();
    orders_.erase(it);
}

OrderPtr PriceLevel::getFrontOrder() const {
    return orders_.empty() ? nullptr : orders_.front();
}