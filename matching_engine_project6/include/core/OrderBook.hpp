
// ===== include/core/OrderBook.hpp =====
#pragma once
#include "core/BookSide.hpp"
#include <unordered_map>
#include <string>

class OrderBook {
private:
    std::string instrument_;
    BidSide bids_;
    AskSide asks_;
    std::unordered_map<OrderId, std::pair<OrderPtr, Price>> orderIndex_;
    
public:
    explicit OrderBook(const std::string& instrument);
    
    void addOrder(OrderPtr order);
    void removeOrder(OrderId orderId);
    OrderPtr findOrder(OrderId orderId) const;
    
    BidSide& getBids() { return bids_; }
    AskSide& getAsks() { return asks_; }
    const std::string& getInstrument() const { return instrument_; }
    
    Price getBestBid() const { return bids_.getBestPrice(); }
    Price getBestAsk() const { return asks_.getBestPrice(); }
};