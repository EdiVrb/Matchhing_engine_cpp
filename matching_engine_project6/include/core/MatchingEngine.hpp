// =====include/core/MatchingEngine.hpp =====
#pragma once
#include "core/OrderBook.hpp"
#include "core/OrderEvent.hpp"
#include "core/Trade.hpp"
#include <memory>
#include <vector>
#include <unordered_map>

class MatchingEngine {
private:
    OrderBook orderBook_;
    std::vector<OrderEvent> events_;  // Remplace executedTrades_
    std::unordered_map<OrderId, OrderPtr> orderHistory_;
    
public:
    explicit MatchingEngine(const std::string& instrument);
    
    void processOrder(Timestamp actionTimestamp, OrderId id,
                     Side side, OrderType type, 
                     Quantity quantity, Price price, 
                     Action action);
    
    const OrderBook& getOrderBook() const { return orderBook_; }
    const std::vector<OrderEvent>& getEvents() const { return events_; }
    OrderPtr getOrder(OrderId id) const;
};
