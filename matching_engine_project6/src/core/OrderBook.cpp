
// ===== src/core/OrderBook.cpp =====
#include "core/OrderBook.hpp"
#include "exceptions/Exceptions.hpp"

OrderBook::OrderBook(const std::string& instrument) 
    : instrument_(instrument) {}

void OrderBook::addOrder(OrderPtr order) {
    Price price = order->getPrice();
    orderIndex_[order->getOrderId()] = {order, price};
    
    if (order->getSide() == Side::BUY) {
        bids_.addOrder(order);
    } else {
        asks_.addOrder(order);
    }
}

void OrderBook::removeOrder(OrderId orderId) {
    auto it = orderIndex_.find(orderId);
    if (it == orderIndex_.end()) {
        throw OrderNotFoundException(orderId);
    }
    
    auto& [order, price] = it->second;
    
    if (order->getSide() == Side::BUY) {
        bids_.removeOrder(orderId, price);
    } else {
        asks_.removeOrder(orderId, price);
    }
    
    orderIndex_.erase(it);
}

OrderPtr OrderBook::findOrder(OrderId orderId) const {
    auto it = orderIndex_.find(orderId);
    return (it != orderIndex_.end()) ? it->second.first : nullptr;
}