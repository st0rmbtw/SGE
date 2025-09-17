#pragma once

#ifndef _SGE_TYPES_ORDER_HPP_
#define _SGE_TYPES_ORDER_HPP_

namespace sge {

struct Order {
    constexpr Order() = default;

    explicit constexpr Order(int order, bool advance = true) noexcept :
        value(order),
        advance(advance) {}

    int value = -1;
    bool advance = true;
};

}

#endif