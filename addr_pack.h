#pragma once

#include <cstdint>

struct __attribute__((packed)) AddressPack {
    uint32_t ip = 0;
    uint16_t port = 0;
};
