// ===== include/core/BookSide.hpp =====
#pragma once
#include "core/PriceLevel.hpp"
#include <map>
#include <memory>

template<typename Comparator>
class BookSide {
private:
    std::map<Price, std::unique_ptr<PriceLevel>, Comparator> levels_;
    
public:
    BookSide() = default;
    
    void addOrder(OrderPtr order) {
        // IMPORTANT: Les ordres MARKET ne doivent JAMAIS être ajoutés au carnet
        if (order->getType() == OrderType::MARKET) {
            return;
        }
        
        Price price = order->getPrice();
        auto it = levels_.find(price);
        
        if (it == levels_.end()) {
            auto level = std::make_unique<PriceLevel>(price);
            level->addOrder(order);
            levels_[price] = std::move(level);
        } else {
            it->second->addOrder(order);
        }
    }
    
    void removeOrder(OrderId orderId, Price price) {
        auto it = levels_.find(price);
        if (it != levels_.end()) {
            it->second->removeOrder(orderId);
            if (it->second->isEmpty()) {
                levels_.erase(it);
            }
        }
    }
    
    PriceLevel* getBestLevel() {
        return levels_.empty() ? nullptr : levels_.begin()->second.get();
    }
    
    Price getBestPrice() const {
        return levels_.empty() ? 0.0 : levels_.begin()->first;
    }
    
    bool isEmpty() const { return levels_.empty(); }
    
    auto begin() { return levels_.begin(); }
    auto end() { return levels_.end(); }
};

using BidSide = BookSide<std::greater<Price>>;
using AskSide = BookSide<std::less<Price>>;