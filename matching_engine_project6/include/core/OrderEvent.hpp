// ===== include/core/OrderEvent.hpp =====
#pragma once
#include "types/OrderTypes.hpp"
#include "types/Enums.hpp"
#include <string>

struct OrderEvent {
    Timestamp actionTimestamp;
    OrderId orderId;
    std::string instrument;
    Side side;
    OrderType type;
    Quantity displayQuantity;  // 0 if EXECUTED, remaining if PARTIALLY_EXECUTED
    Price price;
    Action action;
    OrderStatus status;
    Quantity executedQuantity;
    Price executionPrice;
    OrderId counterpartyId;
    
    OrderEvent(Timestamp ts, OrderId id, const std::string& inst, Side s, 
               OrderType t, Quantity qty, Price p, Action a, OrderStatus st,
               Quantity execQty = 0, Price execPrice = 0.0, OrderId cpId = 0)
        : actionTimestamp(ts), orderId(id), instrument(inst), side(s),
          type(t), displayQuantity(qty), price(p), action(a), status(st),
          executedQuantity(execQty), executionPrice(execPrice), counterpartyId(cpId) {}
};