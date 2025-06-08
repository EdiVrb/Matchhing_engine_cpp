// ===== tests/test_MatchingEngine.cpp =====
#include <gtest/gtest.h>
#include "core/MatchingEngine.hpp"
#include "utils/TimeUtils.hpp"
#include "exceptions/Exceptions.hpp"

class MatchingEngineTest : public ::testing::Test {
protected:
    std::unique_ptr<MatchingEngine> engine;
    
    void SetUp() override {
        engine = std::make_unique<MatchingEngine>("AAPL");
    }
    
    const OrderEvent* findEvent(const std::vector<OrderEvent>& events, 
                               OrderId orderId, OrderStatus status) {
        for (const auto& event : events) {
            if (event.orderId == orderId && event.status == status) {
                return &event;
            }
        }
        return nullptr;
    }
};

TEST_F(MatchingEngineTest, ActionNew) {
    // Test NEW pour ordre LIMIT
    engine->processOrder(1000, 1, Side::BUY, OrderType::LIMIT, 100, 150.00, Action::NEW);
    
    auto& events = engine->getEvents();
    ASSERT_EQ(events.size(), 1);
    
    auto& event = events[0];
    EXPECT_EQ(event.orderId, 1);
    EXPECT_EQ(event.action, Action::NEW);
    EXPECT_EQ(event.status, OrderStatus::PENDING);
    EXPECT_EQ(event.displayQuantity, 100);
    EXPECT_DOUBLE_EQ(event.price, 150.00);
    
    // Vérifier que l'ordre est dans le carnet
    auto order = engine->getOrder(1);
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->getStatus(), OrderStatus::PENDING);
}

TEST_F(MatchingEngineTest, ActionModify) {
    // Créer un ordre
    engine->processOrder(1000, 1, Side::BUY, OrderType::LIMIT, 100, 150.00, Action::NEW);
    
    // Modifier l'ordre
    engine->processOrder(2000, 1, Side::BUY, OrderType::LIMIT, 200, 151.00, Action::MODIFY);
    
    auto& events = engine->getEvents();
    ASSERT_GE(events.size(), 2);
    
    // Chercher l'événement MODIFY
    const OrderEvent* modifyEvent = nullptr;
    for (const auto& event : events) {
        if (event.action == Action::MODIFY) {
            modifyEvent = &event;
            break;
        }
    }
    
    ASSERT_NE(modifyEvent, nullptr);
    EXPECT_EQ(modifyEvent->orderId, 1);
    EXPECT_EQ(modifyEvent->status, OrderStatus::PENDING);
    EXPECT_EQ(modifyEvent->displayQuantity, 200);
    EXPECT_DOUBLE_EQ(modifyEvent->price, 151.00);
    
    // Vérifier que l'ordre a été mis à jour
    auto order = engine->getOrder(1);
    EXPECT_EQ(order->getQuantity(), 200);
    EXPECT_DOUBLE_EQ(order->getPrice(), 151.00);
}

TEST_F(MatchingEngineTest, ActionCancel) {
    // Créer un ordre
    engine->processOrder(1000, 1, Side::BUY, OrderType::LIMIT, 100, 150.00, Action::NEW);
    
    // Annuler l'ordre
    engine->processOrder(2000, 1, Side::BUY, OrderType::LIMIT, 0, 0, Action::CANCEL);
    
    auto& events = engine->getEvents();
    ASSERT_EQ(events.size(), 2);
    
    auto& cancelEvent = events[1];
    EXPECT_EQ(cancelEvent.orderId, 1);
    EXPECT_EQ(cancelEvent.action, Action::CANCEL);
    EXPECT_EQ(cancelEvent.status, OrderStatus::CANCELED);
    EXPECT_EQ(cancelEvent.displayQuantity, 0);
    
    // L'ordre ne devrait plus être dans le carnet actif
    auto orderInBook = engine->getOrderBook().findOrder(1);
    EXPECT_EQ(orderInBook, nullptr);
}

TEST_F(MatchingEngineTest, ModifyOrderNotFound) {
    // Tenter de modifier un ordre inexistant
    EXPECT_THROW(
        engine->processOrder(1000, 999, Side::BUY, OrderType::LIMIT, 100, 150.00, Action::MODIFY),
        OrderNotFoundException
    );
}

TEST_F(MatchingEngineTest, CancelOrderNotFound) {
    // Tenter d'annuler un ordre inexistant
    EXPECT_THROW(
        engine->processOrder(1000, 999, Side::BUY, OrderType::LIMIT, 0, 0, Action::CANCEL),
        OrderNotFoundException
    );
}



TEST_F(MatchingEngineTest, ModifyThenExecute) {
    // Créer un ordre SELL
    engine->processOrder(1000, 1, Side::SELL, OrderType::LIMIT, 100, 150.00, Action::NEW);
    
    // Créer un ordre BUY qui ne matche pas
    engine->processOrder(2000, 2, Side::BUY, OrderType::LIMIT, 100, 149.00, Action::NEW);
    
    // Modifier l'ordre BUY pour qu'il matche
    engine->processOrder(3000, 2, Side::BUY, OrderType::LIMIT, 100, 150.00, Action::MODIFY);
    
    auto& events = engine->getEvents();
    
    // Chercher les événements d'exécution
    bool foundSellExecuted = false;
    bool foundBuyExecuted = false;
    
    for (const auto& event : events) {
        if (event.orderId == 1 && event.status == OrderStatus::EXECUTED) {
            foundSellExecuted = true;
            EXPECT_EQ(event.counterpartyId, 2);
        }
        if (event.orderId == 2 && event.status == OrderStatus::EXECUTED) {
            foundBuyExecuted = true;
            EXPECT_EQ(event.action, Action::MODIFY); // Important: l'action est MODIFY
            EXPECT_EQ(event.counterpartyId, 1);
        }
    }
    
    EXPECT_TRUE(foundSellExecuted);
    EXPECT_TRUE(foundBuyExecuted);
}

TEST_F(MatchingEngineTest, CancelPartiallyExecutedOrder) {
    // Créer un ordre SELL de 50
    engine->processOrder(1000, 1, Side::SELL, OrderType::LIMIT, 50, 150.00, Action::NEW);
    
    // Créer un ordre BUY de 100 qui va partiellement matcher
    engine->processOrder(2000, 2, Side::BUY, OrderType::LIMIT, 100, 150.00, Action::NEW);
    
    // Annuler l'ordre BUY partiellement exécuté
    engine->processOrder(3000, 2, Side::BUY, OrderType::LIMIT, 0, 0, Action::CANCEL);
    
    auto& events = engine->getEvents();
    
    // Vérifier qu'on a bien l'événement CANCEL
    auto cancelEvent = findEvent(events, 2, OrderStatus::CANCELED);
    ASSERT_NE(cancelEvent, nullptr);
    EXPECT_EQ(cancelEvent->action, Action::CANCEL);
    
    // L'ordre ne devrait plus être dans le carnet
    auto orderInBook = engine->getOrderBook().findOrder(2);
    EXPECT_EQ(orderInBook, nullptr);
}