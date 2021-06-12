#pragma once

#include <cstdint>

using DataId = uint32_t;

namespace DataType {

#define DECLARE_DATA_ID(name, id) \
    static constexpr DataId name = id

DECLARE_DATA_ID(UNKNOWN, 0);
DECLARE_DATA_ID(SYS_INFO, 1);
DECLARE_DATA_ID(POWER_STATUS, 2);

} // namespace Id