#pragma once

#ifndef _SGE_TYPES_ORDER_HPP_
#define _SGE_TYPES_ORDER_HPP_

#include "../defines.hpp"

_SGE_BEGIN

namespace types {

struct Order {
    Order() = default;

    explicit Order(int order, bool advance = true) :
        value(order),
        advance(advance) {}

    int value = -1;
    bool advance = true;
};

}

_SGE_END

#endif