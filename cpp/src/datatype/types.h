#pragma once

#include <cstdint>
#include <string>

using namespace std;

struct SysInfo 
{
    bool boot_cnt_loaded;
    uint32_t boot_cnt;
    SysInfo(bool bcl = false, uint32_t bc = 0): 
        boot_cnt_loaded(bcl), 
        boot_cnt(bc) 
        {}
};

struct PowerStatus
{
    bool is_charging;
    uint32_t remain_time_mins;
    uint32_t temperature_deg;

    PowerStatus(bool chrg = false, uint32_t remain_time = 0, uint32_t temperature = 0):
        is_charging(chrg),
        remain_time_mins(remain_time),
        temperature_deg(temperature)
        {}
};

