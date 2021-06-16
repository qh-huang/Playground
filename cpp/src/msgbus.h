#pragma once

#include "dispatcher.h"
#include "msg.h"

#include <mutex>
#include <queue>
#include <unordered_map>

using namespace std;

class MsgBus
{
public:
    class Subscriber : public Dispatcher<MsgPtr>
    {
    public:
        Subscriber(string name): Dispatcher(name + "_msgbus_sub") {}

        void Subscribe(DataId data_id) {
            MsgBus::Subscribe(data_id, this);
        }

        void SubscribeAll() {
            MsgBus::SubscribeAll(this);
        }

        virtual bool ProcMsg(MsgPtr msg) = 0;

        // TODO: should be private    
        bool Process(MsgPtr msg) override {
            return ProcMsg(msg);  
        }
    };

    // TODO: add Publisher
    class Publisher
    {
    public:
        void Publish(MsgPtr msg)
        {
            MsgBus::Publish(msg);
        }
    };


    static void Publish(MsgPtr msg) { Instance().PubMsg(msg); }

    static void Subscribe(DataId data_id, Subscriber* sub) {
        Instance().SubMsg(data_id, sub);
    }

    static void SubscribeAll(Subscriber* sub) {
        Instance().SubAll(sub);
    }

private:
    static MsgBus& Instance() {
        static MsgBus inst;
        return inst;
    }

    void PubMsg(MsgPtr msg) {
        for (auto sub : id_subsribers_map_[msg->data_id]) {
            sub->Dispatch(msg);
        }
        for (auto sub : all_id_subsribers_) {
            sub->Dispatch(msg);
        }
    }

    void SubMsg(DataId data_id, Subscriber* sub) {
        id_subsribers_map_[data_id].push_back(sub);
    }

    void SubAll(Subscriber* sub) {
        all_id_subsribers_.push_back(sub);
    }

    unordered_map<DataId, vector<Subscriber*> > id_subsribers_map_;
    vector<Subscriber*> all_id_subsribers_;
};