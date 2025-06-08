// ===== tests/test_MarketOrders.cpp =====
#include <gtest/gtest.h>
#include "core/Order.hpp"
#include "core/MatchingEngine.hpp"
#include "core/InstrumentManager.hpp"
#include "utils/TimeUtils.hpp"
#include "exceptions/Exceptions.hpp"


class MarketOrderTest : public ::testing::Test {
protected:
    InstrumentManager manager;
    
    void SetUp() override {
        // Créer un carnet d'ordres initial pour les tests
        // SELL side
        manager.processOrder(1000, 1, "AAPL", Side::SELL, OrderType::LIMIT, 100, 150.00, Action::NEW);
        manager.processOrder(1001, 2, "AAPL", Side::SELL, OrderType::LIMIT, 200, 151.00, Action::NEW);
        manager.processOrder(1002, 3, "AAPL", Side::SELL, OrderType::LIMIT, 300, 152.00, Action::NEW);
        
        // BUY side
        manager.processOrder(1003, 4, "AAPL", Side::BUY, OrderType::LIMIT, 150, 149.00, Action::NEW);
        manager.processOrder(1004, 5, "AAPL", Side::BUY, OrderType::LIMIT, 250, 148.00, Action::NEW);
    }
};

TEST_F(MarketOrderTest, PrixForceAZero) {
    // Test que le prix d'un ordre MARKET est toujours 0
    auto order = std::make_shared<Order>(getCurrentTimestamp(), 100, "AAPL", 
                                       Side::BUY, OrderType::MARKET, 100, 999.99);
    
    EXPECT_DOUBLE_EQ(order->getPrice(), 0.0);
    
    // Test avec différentes valeurs de prix en entrée
    auto order2 = std::make_shared<Order>(getCurrentTimestamp(), 101, "AAPL", 
                                        Side::SELL, OrderType::MARKET, 50, -50.0);
    EXPECT_DOUBLE_EQ(order2->getPrice(), 0.0);
}

TEST_F(MarketOrderTest, ExecutionComplete) {
    // Ordre MARKET BUY de 100 unités qui doit s'exécuter complètement @ 150.00
    manager.processOrder(2000, 10, "AAPL", Side::BUY, OrderType::MARKET, 100, 0.0, Action::NEW);
    
    auto events = manager.getAllEvents();
    
    // Chercher les événements de l'ordre #10
    int executedCount = 0;
    double executionPrice = 0.0;
    
    for (const auto& event : events) {
        if (event.orderId == 10 && event.status == OrderStatus::EXECUTED) {
            executedCount++;
            executionPrice = event.executionPrice;
            EXPECT_EQ(event.executedQuantity, 100);
            EXPECT_DOUBLE_EQ(event.price, 0.0); // Prix affiché doit être 0
            EXPECT_DOUBLE_EQ(event.executionPrice, 150.00); // Prix d'exécution réel
        }
    }
    
    EXPECT_EQ(executedCount, 1); // Doit avoir exactement un événement EXECUTED
}

TEST_F(MarketOrderTest, ExecutionPartielleAvecAnnulation) {
    // Ordre MARKET SELL de 500 unités
    // Doit exécuter 150 @ 149.00 + 250 @ 148.00 = 400 unités
    // Et annuler le reliquat de 100 unités
    manager.processOrder(3000, 20, "AAPL", Side::SELL, OrderType::MARKET, 500, 0.0, Action::NEW);
    
    auto events = manager.getAllEvents();
    
    // Compter les événements pour l'ordre #20
    int executedCount = 0;
    int canceledCount = 0;
    Quantity totalExecuted = 0;
    
    for (const auto& event : events) {
        if (event.orderId == 20) {
            if (event.status == OrderStatus::EXECUTED) {
                executedCount++;
                totalExecuted += event.executedQuantity;
                EXPECT_DOUBLE_EQ(event.price, 0.0);
            } else if (event.status == OrderStatus::CANCELED) {
                canceledCount++;
                EXPECT_EQ(event.displayQuantity, 0); // Reliquat annulé
            }
        }
    }
    
    EXPECT_EQ(executedCount, 2); // Un événement EXECUTED pour la partie exécutée
    EXPECT_EQ(canceledCount, 1); // Un événement CANCELED pour le reliquat
    EXPECT_EQ(totalExecuted, 400); // 150 + 250
}

TEST_F(MarketOrderTest, OrdreSansLiquidite) {
    // Créer un nouvel instrument sans liquidité
    manager.processOrder(4000, 30, "MSFT", Side::BUY, OrderType::MARKET, 100, 0.0, Action::NEW);
    
    auto events = manager.getAllEvents();
    
    // L'ordre doit être immédiatement annulé
    bool foundCanceled = false;
    for (const auto& event : events) {
        if (event.orderId == 30) {
            EXPECT_EQ(event.status, OrderStatus::CANCELED);
            EXPECT_EQ(event.executedQuantity, 0);
            EXPECT_DOUBLE_EQ(event.price, 0.0);
            foundCanceled = true;
        }
    }
    
    EXPECT_TRUE(foundCanceled);
}

TEST_F(MarketOrderTest, ModificationOrdreMarketInterdit) {
    // Créer un ordre MARKET
    manager.processOrder(5000, 40, "AAPL", Side::BUY, OrderType::MARKET, 50, 0.0, Action::NEW);
    
    // Tenter de modifier un ordre MARKET devrait échouer
    EXPECT_THROW(
        manager.processOrder(5001, 40, "AAPL", Side::BUY, OrderType::MARKET, 100, 0.0, Action::MODIFY),
        InvalidOrderException
    );
}

TEST_F(MarketOrderTest, ExecutionAvecPlusieursNiveaux) {
    // Ordre MARKET BUY de 350 unités
    // Doit exécuter : 100 @ 150.00 + 200 @ 151.00 + 50 @ 152.00
    manager.processOrder(6000, 50, "AAPL", Side::BUY, OrderType::MARKET, 350, 999.0, Action::NEW);
    
    auto events = manager.getAllEvents();
    
    // Vérifier l'exécution contre plusieurs niveaux de prix
    std::map<Price, Quantity> executions;
    
    for (const auto& event : events) {
        if (event.orderId == 50 && event.status == OrderStatus::EXECUTED) {
            EXPECT_DOUBLE_EQ(event.price, 0.0); // Prix ordre MARKET toujours 0
            EXPECT_EQ(event.executedQuantity, 350);
        }
        // Vérifier les contreparties
        if ((event.orderId == 1 || event.orderId == 2 || event.orderId == 3) && 
            event.counterpartyId == 50) {
            executions[event.executionPrice] = event.executedQuantity;
        }
    }
    
    // Vérifier que l'exécution s'est faite aux bons prix
    EXPECT_EQ(executions[150.00], 100);
    EXPECT_EQ(executions[151.00], 200);
    EXPECT_EQ(executions[152.00], 50);
}