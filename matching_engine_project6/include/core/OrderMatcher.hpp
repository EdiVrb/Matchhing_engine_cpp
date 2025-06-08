
// ===== include/core/OrderMatcher.hpp =====
#pragma once
#include "core/OrderBook.hpp"
#include "core/Trade.hpp"
#include <vector>

class OrderMatcher {
public:
    static std::vector<Trade> matchOrder(OrderPtr incomingOrder, OrderBook& book);
    
private:
    static std::vector<Trade> matchLimitOrder(OrderPtr order, OrderBook& book);
    static std::vector<Trade> matchMarketOrder(OrderPtr order, OrderBook& book);
    
    template<typename BookSideType>
    static std::vector<Trade> matchAgainstSide(OrderPtr incomingOrder, 
                                              BookSideType& bookSide,
                                              OrderBook& book);
};