
// ===== include/core/Trade.hpp =====
#pragma once
#include "types/OrderTypes.hpp"
#include <string>

struct Trade {
    Timestamp timestamp;
    OrderId buyOrderId;
    OrderId sellOrderId;
    std::string instrument;
    Quantity quantity;
    Price price;
    
    Trade(Timestamp ts, OrderId buyId, OrderId sellId, 
          const std::string& inst, Quantity qty, Price p)
        : timestamp(ts), buyOrderId(buyId), sellOrderId(sellId),
          instrument(inst), quantity(qty), price(p) {}
};