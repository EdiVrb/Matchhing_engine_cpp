// ===== tests/test_OrderMatcher.cpp =====
#include <gtest/gtest.h>
#include "core/OrderMatcher.hpp"
#include "core/OrderBook.hpp"
#include "utils/TimeUtils.hpp"
#include "exceptions/Exceptions.hpp"

class OrderMatcherTest : public ::testing::Test {
protected:
    std::unique_ptr<OrderBook> book;
    
    void SetUp() override {
        book = std::make_unique<OrderBook>("AAPL");
    }
    
    OrderPtr createOrder(OrderId id, Side side, OrderType type, 
                        Quantity qty, Price price) {
        return std::make_shared<Order>(getCurrentTimestamp(), id, "AAPL", 
                                     side, type, qty, price);
    }
};

TEST_F(OrderMatcherTest, MatchingLimitOrdersCroisement) {
    // Ajouter des ordres SELL dans le carnet
    auto sell1 = createOrder(1, Side::SELL, OrderType::LIMIT, 100, 150.00);
    auto sell2 = createOrder(2, Side::SELL, OrderType::LIMIT, 200, 151.00);
    book->addOrder(sell1);
    book->addOrder(sell2);
    
    // Ordre BUY qui croise le spread
    auto buy = createOrder(3, Side::BUY, OrderType::LIMIT, 150, 150.50);
    
    auto trades = OrderMatcher::matchOrder(buy, *book);
    
    // Doit matcher 100 @ 150.00 avec ordre #1
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].buyOrderId, 3);
    EXPECT_EQ(trades[0].sellOrderId, 1);
    EXPECT_EQ(trades[0].quantity, 100);
    EXPECT_DOUBLE_EQ(trades[0].price, 150.00);
    
    // Vérifier que l'ordre buy a encore 50 unités restantes
    EXPECT_EQ(buy->getRemainingQuantity(), 50);
    EXPECT_EQ(buy->getStatus(), OrderStatus::PARTIALLY_EXECUTED);
}

TEST_F(OrderMatcherTest, MatchingMarketOrder) {
    // Créer un carnet avec plusieurs niveaux
    book->addOrder(createOrder(1, Side::SELL, OrderType::LIMIT, 100, 150.00));
    book->addOrder(createOrder(2, Side::SELL, OrderType::LIMIT, 200, 151.00));
    book->addOrder(createOrder(3, Side::SELL, OrderType::LIMIT, 300, 152.00));
    
    // Ordre MARKET BUY de 250 unités
    auto marketBuy = createOrder(4, Side::BUY, OrderType::MARKET, 250, 0.0);
    
    auto trades = OrderMatcher::matchOrder(marketBuy, *book);
    
    // Doit matcher : 100 @ 150.00 + 150 @ 151.00
    ASSERT_EQ(trades.size(), 2);
    
    // Premier trade
    EXPECT_EQ(trades[0].buyOrderId, 4);
    EXPECT_EQ(trades[0].sellOrderId, 1);
    EXPECT_EQ(trades[0].quantity, 100);
    EXPECT_DOUBLE_EQ(trades[0].price, 150.00);
    
    // Deuxième trade
    EXPECT_EQ(trades[1].buyOrderId, 4);
    EXPECT_EQ(trades[1].sellOrderId, 2);
    EXPECT_EQ(trades[1].quantity, 150);
    EXPECT_DOUBLE_EQ(trades[1].price, 151.00);
    
    // L'ordre MARKET doit être complètement exécuté
    EXPECT_EQ(marketBuy->getStatus(), OrderStatus::EXECUTED);
}

TEST_F(OrderMatcherTest, PrioritePrixTemps) {
    // Ajouter plusieurs ordres au même prix (priorité temporelle)
    auto sell1 = createOrder(1, Side::SELL, OrderType::LIMIT, 100, 150.00);
    auto sell2 = createOrder(2, Side::SELL, OrderType::LIMIT, 100, 150.00);
    auto sell3 = createOrder(3, Side::SELL, OrderType::LIMIT, 100, 149.00); // Meilleur prix
    
    book->addOrder(sell1);
    book->addOrder(sell2);
    book->addOrder(sell3);
    
    // Ordre BUY qui peut matcher avec tous
    auto buy = createOrder(4, Side::BUY, OrderType::LIMIT, 250, 150.00);
    
    auto trades = OrderMatcher::matchOrder(buy, *book);
    
    ASSERT_EQ(trades.size(), 3);
    
    // Doit d'abord matcher avec le meilleur prix (ordre #3)
    EXPECT_EQ(trades[0].sellOrderId, 3);
    EXPECT_DOUBLE_EQ(trades[0].price, 149.00);
    
    // Puis avec ordre #1 (premier arrivé à 150.00)
    EXPECT_EQ(trades[1].sellOrderId, 1);
    EXPECT_DOUBLE_EQ(trades[1].price, 150.00);
    
    // Enfin avec ordre #2
    EXPECT_EQ(trades[2].sellOrderId, 2);
    EXPECT_DOUBLE_EQ(trades[2].price, 150.00);
    EXPECT_EQ(trades[2].quantity, 50); // Seulement 50 restantes
}

TEST_F(OrderMatcherTest, PasDeMatchingSiPrixNeCroisentPas) {
    // SELL @ 151.00
    book->addOrder(createOrder(1, Side::SELL, OrderType::LIMIT, 100, 151.00));
    
    // BUY @ 150.00 - ne croise pas
    auto buy = createOrder(2, Side::BUY, OrderType::LIMIT, 100, 150.00);
    
    auto trades = OrderMatcher::matchOrder(buy, *book);
    
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(buy->getStatus(), OrderStatus::PENDING);
    
    // L'ordre doit être ajouté au carnet
    auto foundOrder = book->findOrder(2);
    EXPECT_NE(foundOrder, nullptr);
}

TEST_F(OrderMatcherTest, MarketOrderSansLiquidite) {
    // Carnet vide
    auto marketBuy = createOrder(1, Side::BUY, OrderType::MARKET, 100, 0.0);
    
    auto trades = OrderMatcher::matchOrder(marketBuy, *book);
    
    EXPECT_TRUE(trades.empty());
    // L'ordre MARKET doit être annulé
    EXPECT_EQ(marketBuy->getStatus(), OrderStatus::CANCELED);
    EXPECT_EQ(marketBuy->getRemainingQuantity(), 100);
}

TEST_F(OrderMatcherTest, MarketOrderReliquatAnnule) {
    // Seulement 100 unités disponibles
    book->addOrder(createOrder(1, Side::SELL, OrderType::LIMIT, 100, 150.00));
    
    // Ordre MARKET pour 200 unités
    auto marketBuy = createOrder(2, Side::BUY, OrderType::MARKET, 200, 0.0);
    
    auto trades = OrderMatcher::matchOrder(marketBuy, *book);
    
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 100);
    
    // L'ordre doit être annulé avec 100 unités exécutées
    EXPECT_EQ(marketBuy->getStatus(), OrderStatus::CANCELED);
    EXPECT_EQ(marketBuy->getExecutedQuantity(), 100);
    EXPECT_EQ(marketBuy->getRemainingQuantity(), 100);
}