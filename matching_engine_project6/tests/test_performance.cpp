
// ===== tests/test_performance.cpp =====
#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include "core/InstrumentManager.hpp"
#include "utils/TimeUtils.hpp"

class PerformanceTest : public ::testing::Test {
protected:
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<> priceDist{100.0, 200.0};
    std::uniform_int_distribution<> qtyDist{1, 1000};
    std::uniform_int_distribution<> sideDist{0, 1};
};

TEST_F(PerformanceTest, Process100KOrders) {
    InstrumentManager manager;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (OrderId id = 1; id <= 100000; ++id) {
        Side side = sideDist(rng) == 0 ? Side::BUY : Side::SELL;
        Quantity qty = qtyDist(rng);
        Price price = priceDist(rng);
        
        manager.processOrder(getCurrentTimestamp(), id, "AAPL", side, 
                           OrderType::LIMIT, qty, price, Action::NEW);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    EXPECT_LT(duration.count(), 600); // Moins de 10 minutes
    
    std::cout << "Processed 100K orders in " << duration.count() << " seconds\n";
}