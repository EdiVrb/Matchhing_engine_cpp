cmake_minimum_required(VERSION 3.10)
project(MatchingEngine)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Liste des fichiers sources
set(SOURCES
    src/core/Order.cpp
    src/core/OrderBook.cpp
    src/core/OrderMatcher.cpp
    src/core/MatchingEngine.cpp
    src/core/PriceLevel.cpp
    src/core/InstrumentManager.cpp
    src/io/CSVReader.cpp
    src/io/CSVWriter.cpp
    src/utils/Logger.cpp
)

# Executable principal
add_executable(matching_engine main.cpp ${SOURCES})

# Tests avec Google Test
enable_testing()

# Chercher Google Test
find_package(GTest)

if(GTEST_FOUND)
    # Si trouvé via find_package
    include_directories(${GTEST_INCLUDE_DIRS})
    
    # Test Order
    add_executable(test_order tests/test_Order.cpp ${SOURCES})
    target_link_libraries(test_order ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
    
    # Test Market Orders
    add_executable(test_market_orders tests/test_MarketOrders.cpp ${SOURCES})
    target_link_libraries(test_market_orders ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
    
    # Test Order Matcher
    add_executable(test_order_matcher tests/test_OrderMatcher.cpp ${SOURCES})
    target_link_libraries(test_order_matcher ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
    
    # Test Matching Engine
    add_executable(test_matching_engine tests/test_MatchingEngine.cpp ${SOURCES})
    target_link_libraries(test_matching_engine ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
    
    # Test Performance
    add_executable(test_performance tests/test_performance.cpp ${SOURCES})
    target_link_libraries(test_performance ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
else()
    # Sinon, lier directement
    message(STATUS "Using direct linking for Google Test")
    
    # Test Order
    add_executable(test_order tests/test_Order.cpp ${SOURCES})
    target_link_libraries(test_order gtest gtest_main pthread)
    
    # Test Market Orders
    add_executable(test_market_orders tests/test_MarketOrders.cpp ${SOURCES})
    target_link_libraries(test_market_orders gtest gtest_main pthread)
    
    # Test Order Matcher
    add_executable(test_order_matcher tests/test_OrderMatcher.cpp ${SOURCES})
    target_link_libraries(test_order_matcher gtest gtest_main pthread)
    
    # Test Matching Engine
    add_executable(test_matching_engine tests/test_MatchingEngine.cpp ${SOURCES})
    target_link_libraries(test_matching_engine gtest gtest_main pthread)
    
    # Test Performance
    add_executable(test_performance tests/test_performance.cpp ${SOURCES})
    target_link_libraries(test_performance gtest gtest_main pthread)
endif()

# Ajouter les tests pour CTest
add_test(NAME OrderTest COMMAND test_order)
add_test(NAME MarketOrderTest COMMAND test_market_orders)
add_test(NAME OrderMatcherTest COMMAND test_order_matcher)
add_test(NAME MatchingEngineTest COMMAND test_matching_engine)
add_test(NAME PerformanceTest COMMAND test_performance)