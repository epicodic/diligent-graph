#pragma once

#include <cstdint>
#include <ostream>

namespace dg {

union RenderOrder
{
    struct {
        std::uint16_t major_order;
        std::uint16_t minor_order;
    };

    std::uint32_t value = 0;

    RenderOrder() = default;
    RenderOrder(std::uint16_t major, std::uint16_t minor) : major_order(major), minor_order(minor) {}

    bool operator<(RenderOrder other) const { return value < other.value; }

    friend std::ostream& operator<<(std::ostream& os, RenderOrder o)
    {
        os << o.major_order << "." << o.minor_order;
        return os;
    }
};



}
