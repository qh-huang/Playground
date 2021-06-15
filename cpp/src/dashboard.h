#pragma once

#include "msgbus.h"
#include <mutex>

using namespace std;

using Timestamp = uint64_t;

Timestamp GetCurrentTimestampMs() { return 3345678; } // TODO: create time.h for timestamp/duration/..., etc

class TimedData
{
public:
    TimedData(Timestamp ts = 0): timestamp_(ts) {}
    virtual ~TimedData() = default;

    void SetTimestamp(Timestamp ts)
    {
        lock_guard<mutex> lock(mtx_);
        timestamp_ = ts;
    }

    Timestamp GetTimestamp() 
    { 
        lock_guard<mutex> lock(mtx_); 
        return timestamp_; 
    }

protected:
    Timestamp timestamp_;
    mutex mtx_;
};
using TimedDataPtr = shared_ptr<TimedData>;

#define DEFINE_TIMED_DATA(TypeName)             \
struct Timed##TypeName : public TimedData {     \
    TypeName data;                              \
    Timed##TypeName(): TimedData(0) {}          \
    TypeName Get()                              \
    {                                           \
        lock_guard<mutex> lock(mtx_);           \
        return data;                            \
    }                                           \
    void Set(const TypeName& d)                 \
    {                                           \
        lock_guard<mutex> lock(mtx_);           \
        data = d;                               \
        timestamp_ = GetCurrentTimestampMs();   \
    }                                           \
};                                              \
using Timed##TypeName##Ptr = shared_ptr<Timed##TypeName>;


DEFINE_TIMED_DATA(SysInfo)

class Dashboard : public MsgBus::Subscriber 
{
public:
    Dashboard(): MsgBus::Subscriber("dshbrd") 
    {
        // fill the keys that we care about
        id_timed_data_map_[DataType::SYS_INFO] = nullptr;
    }

    template<typename T>
    bool GetData(DataId data_id, T& t) {
        if (!HasData(data_id)) return false;
        switch (data_id) {
        case DataType::SYS_INFO:
            TimedSysInfoPtr timed_data = dynamic_pointer_cast<TimedSysInfo>(id_timed_data_map_[data_id]);
            t = timed_data->Get();
            break;
        default:
            cerr << "UNKNOWN data id (should never get here" << endl;
            return false;
        }
        return true;
    }

    // template<typename T>
    // void SetData(DataId data_id, const T& t);

    bool HasData(DataId data_id) {
        if (id_timed_data_map_.find(data_id) != id_timed_data_map_.end()) return false;
        if (!id_timed_data_map_[data_id]) return false;
        return true;
    }

    // Subscriber::ProcMsg
    bool ProcMsg(MsgPtr msg) override
    {
        switch (msg->data_id) {
        case DataType::SYS_INFO:
            if (!id_timed_data_map_[msg->data_id]) {
                cout << "It's the 1st time receiving " << msg->msg_name;
                id_timed_data_map_[msg->data_id] = make_shared<TimedSysInfo>();
            }
            SysInfoMsgPtr message = dynamic_pointer_cast<SysInfoMsg>(msg);
            TimedSysInfoPtr timed_data = dynamic_pointer_cast<TimedSysInfo>(id_timed_data_map_[msg->data_id]);
            timed_data->Set(message->data);
            break;
        }
        return true;
    }

    unordered_map<DataId, TimedDataPtr> id_timed_data_map_;
};

