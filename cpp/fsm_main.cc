#include "fsm.h"
#include "messenger.h"
#include <iostream>

using namespace std;

using ThreadPtr = shared_ptr<thread>;


struct MsgA : public MsgBase {
  int data;

  MsgA(int d) : MsgBase(MsgId::MSG_A, "MSG_A"), data(d) {}
};

// TODO: Context should always inherit enable_shared_from_this, create class IStateContext for TestNode to inherit from
class TestNode : public MsgHandler, public enable_shared_from_this<TestNode> {
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

  using TestStateMachine = EvStateMachineBase<TestStateId, TestEventId, TestNode>;  

  struct EventA : public TestStateMachine::EventBase {
    int data;
    EventA(int d) : EventBase(TestEventId::A, "EV_A"), data(d) {}
  };

  class IdleState : public TestStateMachine::StateBase
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

  class StartState : public TestStateMachine::StateBase
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

  TestNode(): MsgHandler("test_node") {
    state_machine_ = make_shared<TestStateMachine>("test_node");
  }

  virtual ~TestNode() = default;

  void Activate() 
  {
    state_machine_->Start(make_shared<IdleState>(), this->shared_from_this());
  }

  void Deactivate()
  {
    state_machine_->Stop();
  }

  // MsgHandler::ProcMsg
  void ProcMsg(MsgBasePtr msg) override {
    switch (msg->msg_id) {
    case MsgId::MSG_A: {
      shared_ptr<MsgA> msg_a = dynamic_pointer_cast<MsgA>(msg);
      cout << msg_a->data << endl;
      state_machine_->DispatchEvent(make_shared<EventA>(msg_a->data));
    } 
    break;
    default:
      cout << "ignore msg_type: " << msg->msg_name;
    }
  }

private:
  shared_ptr<TestStateMachine> state_machine_;  

};

int main(int argc, char *argv[]) {
  shared_ptr<TestNode> tn = make_shared<TestNode>();
  // tn->Activate();

  int k = 0;
  while (true) {
    tn->Activate();
    for (int i = 0; i < 10; i++) {
      tn->DispatchMsg(make_shared<MsgA>(k++));
      this_thread::sleep_for(chrono::milliseconds(100));
    }
    tn->Deactivate();
    this_thread::sleep_for(chrono::seconds(1));
  }

  return 0;
}