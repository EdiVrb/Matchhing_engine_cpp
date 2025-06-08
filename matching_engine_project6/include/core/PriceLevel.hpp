// ===== include/core/PriceLevel.hpp =====
#pragma once
#include "core/Order.hpp"
#include <list>
#include <algorithm>

class PriceLevel {
private:
    Price price_;
    std::list<OrderPtr> orders_;
    Quantity totalQuantity_;
    
public:
    explicit PriceLevel(Price price);
    
    void addOrder(OrderPtr order);
    void removeOrder(OrderId orderId);
    OrderPtr getFrontOrder() const;
    
    inline Price getPrice() const { return price_; }
    inline Quantity getTotalQuantity() const { return totalQuantity_; }
    inline bool isEmpty() const { return orders_.empty(); }
    inline size_t getOrderCount() const { return orders_.size(); }
    
    // Iterator access
    auto begin() { return orders_.begin(); }
    auto end() { return orders_.end(); }
};
