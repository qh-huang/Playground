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
    class Subscriber : public Dispatcher<MsgPtr>, public enable_shared_from_this<Subscriber>
    {
    public:
        Subscriber(string name): Dispatcher(name + "_msgbus_sub") {}

        void Subscribe(MsgId msg_id) {
            MsgBus::Subscribe(msg_id, shared_from_this());
        }

        void SubscribeAll() {
            MsgBus::SubscribeAll(shared_from_this());
        }

        virtual bool ProcMsg(MsgPtr msg) = 0;

        // TODO: should be private    
        bool Process(MsgPtr msg) override {
            return ProcMsg(msg);  
        }
    };
    using SubscriberPtr = shared_ptr<Subscriber>;

    // TODO: add Publisher
    class Publisher
    {
    public:
        void Publish(MsgPtr msg)
        {
            MsgBus::Publish(msg);
        }
    };

    static MsgBus& Instance() {
        static MsgBus inst;
        return inst;
    }

    static void Publish(MsgPtr msg) { Instance().PubMsg(msg); }

    static void Subscribe(MsgId msg_id, SubscriberPtr sub) {
        Instance().SubMsg(msg_id, sub);
    }

    static void SubscribeAll(SubscriberPtr sub) {
        Instance().SubAll(sub);
    }

private:
    void PubMsg(MsgPtr msg) {
        for (auto sub : id_subsribers_map_[msg->msg_id]) {
            sub->Dispatch(msg);
        }
        for (auto sub : all_id_subsribers_) {
            sub->Dispatch(msg);
        }
    }

    void SubMsg(MsgId msg_id, SubscriberPtr sub) {
        id_subsribers_map_[msg_id].push_back(sub);
    }

    void SubAll(SubscriberPtr sub) {
        all_id_subsribers_.push_back(sub);
    }

    unordered_map<MsgId, vector<SubscriberPtr> > id_subsribers_map_;
    vector<SubscriberPtr> all_id_subsribers_;
};