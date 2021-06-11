#pragma once

#include "dispatcher.h"
#include "msg.h"

#include <mutex>
#include <queue>
#include <unordered_map>

using namespace std;

class MsgHandler : public Dispatcher<MsgPtr>
{
public:
    MsgHandler(string name): Dispatcher<MsgPtr>(name + "_msghdlr") {}
    // Dispatcher::Process() return true if successfully processed
    bool Process(MsgPtr msg) override {
      return ProcMsg(msg);  
    }

    virtual bool ProcMsg(MsgPtr msg) = 0;
};
using MsgHandlerPtr = shared_ptr<MsgHandler>;

class MsgBus
{
public:
    static MsgBus& Instance() {
        static MsgBus inst;
        return inst;
    }
    static void Publish(MsgPtr msg) { Instance().PubMsg(msg); }

    static void Subscribe(MsgId msg_id, MsgHandlerPtr msg_handler) {
        Instance().SubMsg(msg_id, msg_handler);
    }

private:
    void PubMsg(MsgPtr msg) {
        for (auto handler : id_handlers_map_[msg->msg_id]) {
            handler->Dispatch(msg);
        }
    }
    void SubMsg(MsgId msg_id, MsgHandlerPtr msg_handler) {
        id_handlers_map_[msg_id].push_back(msg_handler);
    }
    unordered_map<MsgId, vector<MsgHandlerPtr> > id_handlers_map_;
};