
// ===== include/types/Enums.hpp =====
#pragma once
#include <cstdint>

enum class Side : uint8_t {
    BUY,
    SELL
};

enum class OrderType : uint8_t {
    LIMIT,
    MARKET
};

enum class Action : uint8_t {
    NEW,
    MODIFY,
    CANCEL
};

enum class OrderStatus : uint8_t {
    PENDING,
    EXECUTED,
    PARTIALLY_EXECUTED,
    CANCELED,
    REJECTED
};