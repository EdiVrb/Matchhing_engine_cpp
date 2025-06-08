// ===== include/core/InstrumentManager.hpp =====
#pragma once
#include "core/MatchingEngine.hpp"
#include <unordered_map>
#include <memory>

class InstrumentManager {
private:
    std::unordered_map<std::string, std::unique_ptr<MatchingEngine>> engines_;
    
public:
    void processOrder(Timestamp timestamp, OrderId id,
                     const std::string& instrument, Side side, 
                     OrderType type, Quantity quantity, 
                     Price price, Action action);
    
    std::vector<OrderEvent> getAllEvents() const;
    
private:
    MatchingEngine& getOrCreateEngine(const std::string& instrument);
};
