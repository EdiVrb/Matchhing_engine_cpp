// ===== src/core/MatchingEngine.cpp =====
#include "core/MatchingEngine.hpp"
#include "core/OrderMatcher.hpp"
#include "exceptions/Exceptions.hpp"

MatchingEngine::MatchingEngine(const std::string& instrument) 
    : orderBook_(instrument) {}

void MatchingEngine::processOrder(Timestamp actionTimestamp, OrderId id,
                                 Side side, OrderType type, 
                                 Quantity quantity, Price price, 
                                 Action action) {
    
    switch (action) {
        case Action::NEW: {
            auto order = std::make_shared<Order>(actionTimestamp, id, 
                orderBook_.getInstrument(), side, type, quantity, price);
            
            orderHistory_[id] = order;
            
            // Essayer de matcher AVANT de créer l'événement
            auto trades = OrderMatcher::matchOrder(order, orderBook_);
            
            // Pour les ordres MARKET, ne pas créer d'événement PENDING
            // car ils sont soit exécutés immédiatement, soit annulés
            if (trades.empty() && order->isActive() && order->getType() == OrderType::LIMIT) {
                events_.emplace_back(actionTimestamp, id, orderBook_.getInstrument(),
                                   side, type, quantity, price, Action::NEW,
                                   OrderStatus::PENDING);
            }
            
            // Pour chaque trade, créer les événements d'exécution
            for (const auto& trade : trades) {
                // Récupérer les ordres impliqués
                auto buyOrder = orderHistory_[trade.buyOrderId];
                auto sellOrder = orderHistory_[trade.sellOrderId];
                
                // Event pour l'ordre de vente
                Quantity sellDisplayQty = sellOrder->getRemainingQuantity() > 0 ? 
                    sellOrder->getRemainingQuantity() : 0;
                OrderStatus sellStatus = sellOrder->getRemainingQuantity() > 0 ? 
                    OrderStatus::PARTIALLY_EXECUTED : OrderStatus::EXECUTED;
                
                events_.emplace_back(actionTimestamp, trade.sellOrderId, 
                                   trade.instrument, Side::SELL, sellOrder->getType(),
                                   sellDisplayQty, sellOrder->getPrice(), Action::NEW,
                                   sellStatus, trade.quantity, trade.price, 
                                   trade.buyOrderId);
                
                // Event pour l'ordre d'achat
                Quantity buyDisplayQty = buyOrder->getRemainingQuantity() > 0 ? 
                    buyOrder->getRemainingQuantity() : 0;
                OrderStatus buyStatus = buyOrder->getRemainingQuantity() > 0 ? 
                    OrderStatus::PARTIALLY_EXECUTED : OrderStatus::EXECUTED;
                
                events_.emplace_back(actionTimestamp, trade.buyOrderId, 
                                   trade.instrument, Side::BUY, buyOrder->getType(),
                                   buyDisplayQty, buyOrder->getPrice(), Action::NEW,
                                   buyStatus, trade.quantity, trade.price, 
                                   trade.sellOrderId);
            }
            
            // Gestion spéciale pour les ordres MARKET avec reliquat annulé
            if (order->getType() == OrderType::MARKET && 
                order->getStatus() == OrderStatus::CANCELED && 
                order->getExecutedQuantity() > 0) {
                // L'ordre MARKET a été partiellement exécuté puis annulé
                // Ajouter un événement CANCELED pour le reliquat
                events_.emplace_back(actionTimestamp, id, orderBook_.getInstrument(),
                                   side, type, 0, 0, Action::NEW,
                                   OrderStatus::CANCELED);
            } else if (order->getType() == OrderType::MARKET && 
                       order->getStatus() == OrderStatus::CANCELED && 
                       order->getExecutedQuantity() == 0) {
                // Ordre MARKET sans aucune exécution - complètement annulé
                events_.emplace_back(actionTimestamp, id, orderBook_.getInstrument(),
                                   side, type, 0, 0, Action::NEW,
                                   OrderStatus::CANCELED);
            }
            break;
        }
        
        case Action::MODIFY: {
            auto existingOrder = orderBook_.findOrder(id);
            if (!existingOrder) {
                throw OrderNotFoundException(id);
            }
            
            // Les ordres MARKET ne peuvent pas être modifiés
            if (existingOrder->getType() == OrderType::MARKET) {
                throw InvalidOrderException(id, "Cannot modify MARKET orders");
            }
            
            // Retirer l'ordre du carnet
            orderBook_.removeOrder(id);
            
            // Mettre à jour l'ordre
            existingOrder->updateQuantity(quantity);
            existingOrder->updatePrice(price);
            
            // Essayer de matcher
            auto trades = OrderMatcher::matchOrder(existingOrder, orderBook_);
            
            // Si l'ordre modifié n'a pas été exécuté, créer un événement PENDING
            if (trades.empty() && existingOrder->isActive()) {
                events_.emplace_back(actionTimestamp, id, orderBook_.getInstrument(),
                                   existingOrder->getSide(), existingOrder->getType(),
                                   quantity, price, Action::MODIFY,
                                   OrderStatus::PENDING);
            }
            
            // Générer les événements d'exécution
            for (const auto& trade : trades) {
                auto buyOrder = orderHistory_[trade.buyOrderId];
                auto sellOrder = orderHistory_[trade.sellOrderId];
                
                // Event pour l'ordre de vente
                Quantity sellDisplayQty = sellOrder->getRemainingQuantity() > 0 ? 
                    sellOrder->getRemainingQuantity() : 0;
                OrderStatus sellStatus = sellOrder->getRemainingQuantity() > 0 ? 
                    OrderStatus::PARTIALLY_EXECUTED : OrderStatus::EXECUTED;
                
                Action sellAction = (trade.sellOrderId == id) ? Action::MODIFY : Action::NEW;
                
                events_.emplace_back(actionTimestamp, trade.sellOrderId, 
                                   trade.instrument, Side::SELL, sellOrder->getType(),
                                   sellDisplayQty, sellOrder->getPrice(), sellAction,
                                   sellStatus, trade.quantity, trade.price, 
                                   trade.buyOrderId);
                
                // Event pour l'ordre d'achat
                Quantity buyDisplayQty = buyOrder->getRemainingQuantity() > 0 ? 
                    buyOrder->getRemainingQuantity() : 0;
                OrderStatus buyStatus = buyOrder->getRemainingQuantity() > 0 ? 
                    OrderStatus::PARTIALLY_EXECUTED : OrderStatus::EXECUTED;
                
                Action buyAction = (trade.buyOrderId == id) ? Action::MODIFY : Action::NEW;
                
                events_.emplace_back(actionTimestamp, trade.buyOrderId, 
                                   trade.instrument, Side::BUY, buyOrder->getType(),
                                   buyDisplayQty, buyOrder->getPrice(), buyAction,
                                   buyStatus, trade.quantity, trade.price, 
                                   trade.sellOrderId);
            }
            break;
        }
        
        case Action::CANCEL: {
            auto order = orderBook_.findOrder(id);
            if (!order) {
                // Peut-être déjà exécuté, vérifier dans l'historique
                auto it = orderHistory_.find(id);
                if (it != orderHistory_.end()) {
                    order = it->second;
                } else {
                    throw OrderNotFoundException(id);
                }
            }
            
            if (order->isActive()) {
                order->cancel();
                orderBook_.removeOrder(id);
            }
            
            // Ajouter l'événement d'annulation
            events_.emplace_back(actionTimestamp, id, orderBook_.getInstrument(),
                               order->getSide(), order->getType(),
                               0, 0, Action::CANCEL,
                               OrderStatus::CANCELED);
            break;
        }
    }
}

OrderPtr MatchingEngine::getOrder(OrderId id) const {
    auto it = orderHistory_.find(id);
    return (it != orderHistory_.end()) ? it->second : nullptr;
}