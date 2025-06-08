// ===== src/core/InstrumentManager.cpp =====
#include "core/InstrumentManager.hpp"

void InstrumentManager::processOrder(Timestamp timestamp, OrderId id,
                                   const std::string& instrument, Side side, 
                                   OrderType type, Quantity quantity, 
                                   Price price, Action action) {
    auto& engine = getOrCreateEngine(instrument);
    engine.processOrder(timestamp, id, side, type, quantity, price, action);
}

MatchingEngine& InstrumentManager::getOrCreateEngine(const std::string& instrument) {
    auto it = engines_.find(instrument);
    if (it == engines_.end()) {
        auto result = engines_.emplace(instrument, 
            std::make_unique<MatchingEngine>(instrument));
        return *result.first->second;
    }
    return *it->second;
}

std::vector<OrderEvent> InstrumentManager::getAllEvents() const {
    std::vector<OrderEvent> allEvents;
    
    // Collecter tous les événements de tous les instruments
    for (const auto& [instrument, engine] : engines_) {
        const auto& events = engine->getEvents();
        allEvents.insert(allEvents.end(), events.begin(), events.end());
    }
    
    // Trier par timestamp pour maintenir l'ordre chronologique
    std::sort(allEvents.begin(), allEvents.end(), 
              [](const OrderEvent& a, const OrderEvent& b) {
                  if (a.actionTimestamp != b.actionTimestamp) {
                      return a.actionTimestamp < b.actionTimestamp;
                  }
                  // Si même timestamp, maintenir l'ordre d'insertion
                  return false;
              });
    
    return allEvents;
}
