
#include <gtest/gtest.h>
#include "core/Order.hpp"
#include "exceptions/Exceptions.hpp"
#include "utils/TimeUtils.hpp"

class OrderTest : public ::testing::Test {
protected:
    OrderPtr createTestOrder(OrderId id, const std::string& instrument, 
                           Side side, OrderType type, Quantity qty, Price price) {
        return std::make_shared<Order>(getCurrentTimestamp(), id, instrument, 
                                     side, type, qty, price);
    }
};

TEST_F(OrderTest, CreateValidOrder) {
    auto order = createTestOrder(1, "AAPL", Side::BUY, OrderType::LIMIT, 100, 150.25);
    
    EXPECT_EQ(order->getOrderId(), 1);
    EXPECT_EQ(order->getInstrument(), "AAPL");
    EXPECT_EQ(order->getSide(), Side::BUY);
    EXPECT_EQ(order->getType(), OrderType::LIMIT);
    EXPECT_EQ(order->getQuantity(), 100);
    EXPECT_EQ(order->getRemainingQuantity(), 100);
    EXPECT_DOUBLE_EQ(order->getPrice(), 150.25);
    EXPECT_EQ(order->getStatus(), OrderStatus::PENDING);
}

TEST_F(OrderTest, CreateOrderWithInvalidId) {
    EXPECT_THROW(
        createTestOrder(0, "AAPL", Side::BUY, OrderType::LIMIT, 100, 150.25),
        InvalidOrderException
    );
}

TEST_F(OrderTest, ExecuteOrder) {
    auto order = createTestOrder(1, "AAPL", Side::BUY, OrderType::LIMIT, 100, 150.25);
    
    order->execute(50, 150.25, 2);
    EXPECT_EQ(order->getRemainingQuantity(), 50);
    EXPECT_EQ(order->getStatus(), OrderStatus::PARTIALLY_EXECUTED);
    
    order->execute(50, 150.25, 3);
    EXPECT_EQ(order->getRemainingQuantity(), 0);
    EXPECT_EQ(order->getStatus(), OrderStatus::EXECUTED);
}

TEST_F(OrderTest, CancelOrder) {
    auto order = createTestOrder(1, "AAPL", Side::BUY, OrderType::LIMIT, 100, 150.25);
    
    order->cancel();
    EXPECT_EQ(order->getStatus(), OrderStatus::CANCELED);
}
