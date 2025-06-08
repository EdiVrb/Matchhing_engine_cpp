// ===== src/core/OrderMatcher.cpp =====
#include "core/OrderMatcher.hpp"
#include "utils/TimeUtils.hpp"

std::vector<Trade> OrderMatcher::matchOrder(OrderPtr incomingOrder, OrderBook& book) {
    if (incomingOrder->getType() == OrderType::MARKET) {
        return matchMarketOrder(incomingOrder, book);
    } else {
        return matchLimitOrder(incomingOrder, book);
    }
}

std::vector<Trade> OrderMatcher::matchLimitOrder(OrderPtr order, OrderBook& book) {
    std::vector<Trade> trades;
    
    if (order->getSide() == Side::BUY) {
        auto& asks = book.getAsks();
        trades = matchAgainstSide(order, asks, book);
    } else {
        auto& bids = book.getBids();
        trades = matchAgainstSide(order, bids, book);
    }
    
    if (order->isActive()) {
        book.addOrder(order);
    }
    
    return trades;
}

std::vector<Trade> OrderMatcher::matchMarketOrder(OrderPtr order, OrderBook& book) {
    std::vector<Trade> trades;
    
    if (order->getSide() == Side::BUY) {
        auto& asks = book.getAsks();
        trades = matchAgainstSide(order, asks, book);
    } else {
        auto& bids = book.getBids();
        trades = matchAgainstSide(order, bids, book);
    }
    
    // IMPORTANT: Annuler le reliquat des ordres MARKET non complètement exécutés
    if (order->getRemainingQuantity() > 0) {
        order->cancel();
    }
    
    return trades;
}

template<typename BookSideType>
std::vector<Trade> OrderMatcher::matchAgainstSide(OrderPtr incomingOrder, 
                                                 BookSideType& bookSide,
                                                 OrderBook& book) {
    std::vector<Trade> trades;
    std::vector<OrderId> ordersToRemove; // Pour éviter de modifier pendant l'itération
    
    auto it = bookSide.begin();
    while (it != bookSide.end() && incomingOrder->getRemainingQuantity() > 0) {
        PriceLevel* level = it->second.get();
        Price levelPrice = level->getPrice();
        
        // Vérifier si les prix se croisent
        bool priceMatch = false;
        if (incomingOrder->getType() == OrderType::MARKET) {
            priceMatch = true; // Les ordres MARKET matchent à n'importe quel prix
        } else if (incomingOrder->getSide() == Side::BUY) {
            priceMatch = incomingOrder->getPrice() >= levelPrice;
        } else {
            priceMatch = incomingOrder->getPrice() <= levelPrice;
        }
        
        if (!priceMatch) break;
        
        // Matcher contre les ordres du niveau
        auto orderIt = level->begin();
        while (orderIt != level->end() && incomingOrder->getRemainingQuantity() > 0) {
            OrderPtr bookOrder = *orderIt;
            
            Quantity matchQty = std::min(incomingOrder->getRemainingQuantity(), 
                                       bookOrder->getRemainingQuantity());
            
            // Exécuter les deux ordres au prix du niveau (prix du book)
            incomingOrder->execute(matchQty, levelPrice, bookOrder->getOrderId());
            bookOrder->execute(matchQty, levelPrice, incomingOrder->getOrderId());
            
            // Créer le trade
            Trade trade(
                getCurrentTimestamp(),
                incomingOrder->getSide() == Side::BUY ? incomingOrder->getOrderId() : bookOrder->getOrderId(),
                incomingOrder->getSide() == Side::SELL ? incomingOrder->getOrderId() : bookOrder->getOrderId(),
                book.getInstrument(),
                matchQty,
                levelPrice
            );
            trades.push_back(trade);
            
            // Marquer l'ordre pour suppression s'il est complètement exécuté
            if (!bookOrder->isActive()) {
                ordersToRemove.push_back(bookOrder->getOrderId());
            }
            
            ++orderIt;
        }
        
        ++it;
    }
    
    // Supprimer les ordres exécutés après l'itération
    for (OrderId orderId : ordersToRemove) {
        book.removeOrder(orderId);
    }
    
    return trades;
}

// Instanciation explicite des templates
template std::vector<Trade> OrderMatcher::matchAgainstSide<BidSide>(
    OrderPtr, BidSide&, OrderBook&);
template std::vector<Trade> OrderMatcher::matchAgainstSide<AskSide>(
    OrderPtr, AskSide&, OrderBook&);