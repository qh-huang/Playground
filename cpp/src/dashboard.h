#pragma once

#include "msgbus.h"

template <typename T>
class TimedMsgBase : public MsgBase;
{
public:
    T Get()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return data_;
    }

    Timestamp GetTimestamp()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return timestamp_;
    }

    void Set(const T& data)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_ = data;
        timestamp_ = GetCurrentTimestamp();
    }

private:
    std::mutex mtx_;
    T data_;
    Timestamp timestamp_;
};

#define DEFINE_TIMED_DATA(TypeName, member_)                                    \
public:                                                                         \
    TypeName Get##TypeName() { return member_.Get(); }                          \
    void Set##TypeName(const TypeName&) { member_.Set(x); }                     \
    Timestamp Get##TypeName##Timestamp() { return member_.GetTimestamp(); }     \
private:                                                                        \
    TimedData<TypeName> member_

class Dashboard : public MsgBus::Subscriber 
{
    template<typename T>
    bool GetData(DataId msg_id, T& t);

    template<typename T>
    void SetData(DataId msg_id, const T& t);

    bool HasData(DataId msg_id);
};

