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

        virtual bool ProcMsg(MsgPtr msg) = 0;

        // TODO: should be private    
        bool Process(MsgPtr msg) override {
            return ProcMsg(msg);  
        }
    };
    using SubscriberPtr = shared_ptr<Subscriber>;

    // TODO: add Publisher

    static MsgBus& Instance() {
        static MsgBus inst;
        return inst;
    }
    static void Publish(MsgPtr msg) { Instance().PubMsg(msg); }

    static void Subscribe(MsgId msg_id, SubscriberPtr sub) {
        Instance().SubMsg(msg_id, sub);
    }

private:
    void PubMsg(MsgPtr msg) {
        for (auto handler : id_subsribers_map_[msg->msg_id]) {
            handler->Dispatch(msg);
        }
    }
    void SubMsg(MsgId msg_id, SubscriberPtr sub) {
        id_subsribers_map_[msg_id].push_back(sub);
    }
    unordered_map<MsgId, vector<SubscriberPtr> > id_subsribers_map_;
};