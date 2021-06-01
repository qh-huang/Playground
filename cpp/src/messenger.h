#pragma once

#include "looper.h"
#include <queue>

using MsgIdType = uint32_t;

namespace MsgId
{
  constexpr MsgIdType MSG_UNKNOWN = 0;
  constexpr MsgIdType MSG_A = 1;
}

struct MsgBase {
  const string msg_name;
  const MsgIdType msg_id;

  MsgBase(MsgIdType id, string name) : msg_id(id), msg_name(name) {}
  virtual ~MsgBase() {}
};
using MsgBasePtr = shared_ptr<MsgBase>;

class MsgHandler : public Looper {
public:
  MsgHandler(string name): Looper(name) 
  {
      Looper::Activate();
  }

  virtual ~MsgHandler() = default;

  void DispatchMsg(MsgBasePtr msg) {
    lock_guard<mutex> lock(mtx_queue_);
    msg_queue_.push(msg);
    Looper::Notify();
  }

protected:
  // TODO: make it a pure virtual function
  virtual void ProcMsg(MsgBasePtr msg) = 0;

private:
  // Looper::SpinOnce
  void SpinOnce() override
  {
    // TODO: should use a inner while loop here?
    MsgBasePtr msg = nullptr;
    mtx_queue_.lock();
    if (!msg_queue_.empty()) {
      msg = msg_queue_.front();
      msg_queue_.pop();
    }
    mtx_queue_.unlock();

    ProcMsg(msg);
  }
  bool WaitCondition() override
  {
      return !msg_queue_.empty();
  }

  queue<MsgBasePtr> msg_queue_;
  mutex mtx_queue_;
};
