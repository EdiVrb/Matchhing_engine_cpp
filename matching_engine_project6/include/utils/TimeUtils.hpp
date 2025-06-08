// ===== include/utils/TimeUtils.hpp =====
#pragma once
#include <chrono>
#include "types/OrderTypes.hpp"

inline Timestamp getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}