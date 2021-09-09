#pragma once

#include "dispatcher.h"
#include <queue>

using namespace std;

template<typename StateIdEnumT>
class StateMachineBase {
public:
    class StateBase
    {
    public:
        StateBase(StateIdEnumT sid, const string name): st_id(sid), st_name(name) {}
        virtual ~StateBase() = default;

        string GetName() const { return st_name; }

        virtual void ActionEntry() { cout << "defualt ActionEntry()" << endl; }
        virtual void ActionExit() = 0;
        // TODO: bool ActionGuard()?

        void TransitTo(shared_ptr<StateBase> st) 
        {
            cout << "state transition: " << GetName() << " ==> " << st->GetName() << endl;
            state_machine_->TransitTo(st);
        }

        friend class StateMachineBase<StateIdEnumT>;
    protected:
        static StateMachineBase<StateIdEnumT>* state_machine_;
    private:
        const string st_name;
        const StateIdEnumT st_id;
    };

    StateMachineBase(string name): st_(nullptr) {
        StateBase::state_machine_ = this;
    }

    void Start(shared_ptr<StateBase> init_state) 
    {
        st_ = init_state;
        st_->ActionEntry();
    }

    void Stop()
    {
        st_->ActionExit();
        st_.reset();
    }

    virtual ~StateMachineBase()
    {
        Stop();
    }

    void TransitTo(shared_ptr<StateBase> st) {
        if (st->GetName() == st_->GetName()) {
            // warning: ignore same state transition
            return;
        }
        st_->ActionExit();
        st_ = st;
        st_->ActionEntry();
    }

    shared_ptr<StateBase> GetState() { return st_; }
protected:
    shared_ptr<StateBase> st_;
};
template<typename StateIdEnumT> StateMachineBase<StateIdEnumT>*  StateMachineBase<StateIdEnumT>::StateBase::state_machine_ = nullptr;

// TODO: create a Dispatcher<Object>: public Looper {} to process msg/event asynchronously
template<typename StateIdEnumT, typename EventIdEnumT>
class EvStateMachineBase : public StateMachineBase<StateIdEnumT> {
public:
    struct EventBase
    {
        EventBase(const EventIdEnumT eid, const string name): ev_id(eid), ev_name(name) {}
        virtual ~EventBase() = default;
        const string ev_name;
        const EventIdEnumT ev_id;
    };

    class EvStateBase : public StateMachineBase<StateIdEnumT>::StateBase
    {
    public:
        EvStateBase(StateIdEnumT sid, const string name): StateMachineBase<StateIdEnumT>::StateBase(sid, name) {}
        virtual ~EvStateBase() = default;

        virtual void OnEvent(shared_ptr<EventBase> ev) = 0;
    };

    class EvHandler : public Dispatcher<shared_ptr<EventBase> >
    {
    public:
        EvHandler(string name, EvStateMachineBase* state_machine): Dispatcher<shared_ptr<EventBase> >(name + "_ev_handler"), state_machine_(state_machine) {}
        bool Process(shared_ptr<EventBase> ev) {
            auto st = state_machine_->GetState();
            if (!st) {
                cerr << "st is null" << endl;
                exit(1);
                return false;
            }
            st->OnEvent(ev);
            return true;
        }
    private:
        EvStateMachineBase* state_machine_;
    };

    EvStateMachineBase(string name): StateMachineBase<StateIdEnumT>(name), ev_handler_(name, this)
    {
        // ev_handler_ = make_shared<EvHandler>(name, this);
    }

    virtual ~EvStateMachineBase() = default;

    void EmitEvent(shared_ptr<EventBase> ev) 
    {
        cout << "emitting event: " << ev->ev_name << endl;
        ev_handler_.Dispatch(ev);
    }

    shared_ptr<EvStateBase> GetState() {
        auto st = StateMachineBase<StateIdEnumT>::GetState();
        if (!st) {
            return nullptr;
        }
        return dynamic_pointer_cast<EvStateBase>(st);
    }

private:
    EvHandler ev_handler_;
};

