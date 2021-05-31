#include "fsm.h"
#include <iostream>

using namespace std;

enum class MsgType {
  MSG_A,
  MSG_B,
  MSG_C,
};

ostream &operator<<(ostream &out, const MsgType &msg_type) {
  switch (msg_type) {
  case MsgType::MSG_A:
    out << "A";
    break;
  }
  return out;
}

struct MsgBase {
  const string msg_name;
  const MsgType msg_type;

  MsgBase(MsgType type, string name) : msg_type(type), msg_name(name) {}
  virtual ~MsgBase() {}
};
using MsgBasePtr = shared_ptr<MsgBase>;

using ThreadPtr = shared_ptr<thread>;

class Messenger {
public:
  Messenger() {
    thread_run_ = true;
    worker_thread_ = make_shared<thread>(&Messenger::ThreadLoop, this);
  }

  virtual ~Messenger() {
    thread_run_ = false;
    cv_.notify_one();
    if (worker_thread_) {
      worker_thread_->join();
      worker_thread_.reset();
    }
  }

  void PushAndNotify(MsgBasePtr msg) {
    lock_guard<mutex> lock(mtx_queue_);
    msg_queue_.push(msg);
    cv_.notify_one();
    // cout << __LINE__ << endl;
  }

protected:
  // TODO: make it a pure virtual function
  virtual void ProcMsg(MsgBasePtr msg) = 0;

private:
  void ThreadLoop() {
    while (thread_run_) {
      // cout << __LINE__ << endl;
      unique_lock<mutex> lk(mtx_cv_);
      cout << "start waiting..." << endl;
      cv_.wait(lk, [this] { return !thread_run_ || msg_queue_.size() != 0; });
      cout << "stop waiting!" << endl;
      lk.unlock();
      // cout << __LINE__ << endl;
      // handle corner case that thread_run_ was toggled on other thread
      if (!thread_run_)
        break;

      // TODO: should use a inner while loop here?
      mtx_queue_.lock();
      // cout << __LINE__ << endl;
      MsgBasePtr msg = msg_queue_.front();
      msg_queue_.pop();
      mtx_queue_.unlock();
      // cout << __LINE__ << endl;

      ProcMsg(msg);

      this_thread::sleep_for(chrono::milliseconds(10));
    }
    // warning: exiting ThreadLoop
  }

  queue<MsgBasePtr> msg_queue_;
  mutex mtx_queue_;

  bool thread_run_;
  ThreadPtr worker_thread_;
  mutex mtx_cv_;
  condition_variable cv_;
};

struct MsgA : public MsgBase {
  int data;

  MsgA(int d) : MsgBase(MsgType::MSG_A, "MSG_A"), data(d) {}
};

// enum class TestEventId
// {
//     A
// };

// enum class TestStateId
// {
//     IDLE,
//     START
// };

class TestNode : public Messenger {
public:
  enum class TestEventId
  {
      A
  };

  enum class TestStateId
  {
      IDLE,
      START
  };

  using TestStateMachine = StateMachineBase<TestStateId, TestEventId, TestNode>;  

  struct EventA : public TestStateMachine::EventBase {
    int data;
    EventA(int d) : EventBase(TestEventId::A, "EV_A"), data(d) {}
  };

  class IdleState : public TestStateMachine::StateBase //<TestStateId, TestEventId>
  {
  public:
    IdleState() : StateBase(TestStateId::IDLE, "IDLE") {}
    void OnEvent(shared_ptr<TestStateMachine::EventBase> ev) override {
      switch (ev->ev_id) {
      case TestEventId::A: {
        cout << ev->ev_name << endl;
        shared_ptr<EventA> event_a = dynamic_pointer_cast<EventA>(ev);
        cout << "data: " << event_a->data << endl;
        TransitTo(make_shared<StartState>(event_a->data));
      } break;
      default:
        cout << "IdleState ignores event: " << ev->ev_name << endl;
      }
    }
    void ActionEntry() override 
    { 
        cout << "entering IdleState" << endl; 
    }
    void ActionExit() override { cout << "exiting IdleState" << endl; }
  };

  class StartState : public TestStateMachine::StateBase //<TestStateId, TestEventId>
  {
  public:
    StartState(int d) : StateBase(TestStateId::START, "START"), data(d) {}
    void OnEvent(std::shared_ptr<TestStateMachine::EventBase> ev) override {
      switch (ev->ev_id) {
      case TestEventId::A: {
        cout << ev->ev_name << endl;
        shared_ptr<EventA> event_a = dynamic_pointer_cast<EventA>(ev);
        cout << "data: " << event_a->data << endl;
        TransitTo(make_shared<IdleState>());
      } break;
      default:
        cout << "StartState ignores event: " << ev->ev_name << endl;
      }
    }
    void ActionEntry() override {
      cout << "entering StartState: data(" << data << ")" << endl;
    }
    void ActionExit() override { cout << "exiting StartState" << endl; }

  private:
    int data;
  };

  TestNode() {
      state_machine_ = make_shared<TestStateMachine>();
  }

  virtual ~TestNode() = default;

  void Activate() 
  {
      state_machine_->Start(make_shared<IdleState>(), this);
  }

  void ProcMsg(MsgBasePtr msg) override {
    switch (msg->msg_type) {
    case MsgType::MSG_A: {
      shared_ptr<MsgA> msg_a = dynamic_pointer_cast<MsgA>(msg);
      cout << msg_a->data << endl;
      state_machine_->DispatchEvent(make_shared<EventA>(msg_a->data));
    } break;
    default:
      cout << "ignore msg_type: " << msg->msg_name;
    }
  }

private:
  shared_ptr<TestStateMachine> state_machine_;  

};

int main(int argc, char *argv[]) {
  TestNode tn;
  tn.Activate();

  int k = 0;
  while (true) {
    for (int i = 0; i < 10; i++) {
      tn.PushAndNotify(make_shared<MsgA>(k++));
    }
    this_thread::sleep_for(chrono::seconds(1));
  }

  return 0;
}