#pragma once

#include <cstdint>
#include <string>

using namespace std;

#define DEFINE_PRINT_FUNC_BEGIN(TypeName) \
    void Print(TypeName d) { \
        cout << "+++++ " << #TypeName << " +++++" << endl; 

#define PRINT_VAL(data) \
        do { \
            cout << "\t" << #data << " (" << d.data << ")" << endl; \
        } while (0);

#define DEFINE_PRINT_FUNC_END(TypeName) \
        cout << "----- " << #TypeName << " -----" << endl; \
    }

struct SysInfo 
{
    bool boot_cnt_loaded;
    uint32_t boot_cnt;
    SysInfo(bool bcl = false, uint32_t bc = 0): 
        boot_cnt_loaded(bcl), 
        boot_cnt(bc) 
        {}
};

DEFINE_PRINT_FUNC_BEGIN(SysInfo)
    PRINT_VAL(boot_cnt_loaded)
    PRINT_VAL(boot_cnt)
DEFINE_PRINT_FUNC_END(SysInfo)


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

DEFINE_PRINT_FUNC_BEGIN(PowerStatus)
    PRINT_VAL(is_charging)
    PRINT_VAL(remain_time_mins)
    PRINT_VAL(temperature_deg)
DEFINE_PRINT_FUNC_END(PowerStatus)