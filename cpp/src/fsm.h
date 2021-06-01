#pragma once

#include "looper.h"
#include <queue>

using namespace std;


template<typename StateIdEnumT, typename EventIdEnumT, class ContextT>
class StateMachineBase : public Looper, public enable_shared_from_this<StateMachineBase<StateIdEnumT, EventIdEnumT, ContextT> > {
public:

    struct EventBase
    {
        EventBase(const EventIdEnumT eid, const string name): ev_id(eid), ev_name(name) {}
        virtual ~EventBase() = default;
        const string ev_name;
        const EventIdEnumT ev_id;
    };

    class StateBase
    {
    public:
        StateBase(StateIdEnumT sid, const string name): st_id(sid), st_name(name) {}
        virtual ~StateBase() = default;

        string GetName() const { return st_name; }

        virtual void OnEvent(shared_ptr<EventBase> ev) = 0;
        virtual void ActionEntry() = 0;
        virtual void ActionExit() = 0;
        // TODO: bool ActionGuard()?

        void TransitTo(shared_ptr<StateBase> st) 
        {
            if (!state_machine_) {
                cerr << "state_machine_ is null" << endl;
                return;
            } 

            cout << "state transition: " << GetName() << " ==> " << st->GetName() << endl;
            state_machine_->TransitTo(st);        
        }

        friend class StateMachineBase<StateIdEnumT, EventIdEnumT, ContextT>;

    protected:
        static shared_ptr<ContextT> ctx_;

    private:
        const string st_name;
        const StateIdEnumT st_id;

        static shared_ptr<StateMachineBase<StateIdEnumT, EventIdEnumT, ContextT> > state_machine_;
    };

    StateMachineBase(string name): Looper(name), sm_name_(name), st_(nullptr) 
    {
        Looper::Activate();
    }

    void Start(shared_ptr<StateBase> init_state, ContextT* ctx) 
    {
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
        StateBase::state_machine_ = this->shared_from_this();
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
        StateBase::ctx_ = shared_ptr<ContextT>(ctx);
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
        st_ = init_state;
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
        st_->ActionEntry();
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
    }

    void Stop()
    {
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
        st_->ActionExit();
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
        st_.reset();
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
        // StateBase::ctx_.reset();
        cout << __FILE__ << ": " << __LINE__ << ":" << __FUNCTION__ << endl;
    }

    virtual ~StateMachineBase()
    {
        Stop();
    }

    void DispatchEvent(shared_ptr<EventBase> ev) 
    {
        cout << "pushing event: " << ev->ev_name << endl;
        ev_queue_.push(ev);
        Looper::Notify();
    }

    void ProcessEvent(shared_ptr<EventBase> ev) {
        if (!st_) {
            cerr << "st_ is null" << endl;
            return;
        } 

        st_->OnEvent(ev);
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

private:
    // Looper::SpinOnce
    void SpinOnce() override 
    {
        shared_ptr<EventBase> ev = nullptr;
        mtx_ev_queue_.lock();
        if (!ev_queue_.empty()) {
            ev = ev_queue_.front();
            ev_queue_.pop();
        }
        mtx_ev_queue_.unlock();

        if (ev) {
            ProcessEvent(ev);
        }
    }
    
    // Looper::WaitCondition
    bool WaitCondition() override 
    {
        return !ev_queue_.empty();
    }


    queue<shared_ptr<EventBase> > ev_queue_;
    mutex mtx_ev_queue_;

    shared_ptr<StateBase> st_;

    const string sm_name_;
};
template<typename StateIdEnumT, typename EventIdEnumT, class ContextT> shared_ptr<StateMachineBase<StateIdEnumT, EventIdEnumT, ContextT> > StateMachineBase<StateIdEnumT, EventIdEnumT, ContextT>::StateBase::state_machine_ = nullptr;
template<typename StateIdEnumT, typename EventIdEnumT, class ContextT> shared_ptr<ContextT> StateMachineBase<StateIdEnumT, EventIdEnumT, ContextT>::StateBase::ctx_;


// template<typename StateIdEnumT, typename EventIdEnumT>
// class StateBase
// {
// public:
//     static shared_ptr<StateMachineBase<StateIdEnumT, EventIdEnumT> > state_machine_;

//     StateBase(StateIdEnumT sid, const string name, shared_ptr<StateMachineBase<StateIdEnumT, EventIdEnumT> > st_machine = nullptr): st_id(sid), st_name(name)
//     {
//         cerr << __FILE__ << ": " << __LINE__ << endl;
//         if (st_machine) {
//             state_machine_ = st_machine;
//         }
//     }
//     virtual ~StateBase() = default;

//     string GetName() const { return st_name; }

//     virtual void OnEvent(shared_ptr<EventBase<EventIdEnumT> > ev) = 0;
//     virtual void ActionEntry() = 0;
//     virtual void ActionExit() = 0;
//     // TODO: bool ActionGuard()?

//     void TransitTo(shared_ptr<StateBase<StateIdEnumT, EventIdEnumT> > st) 
//     {
//         if (!state_machine_) {
//             cerr << "state_machine_ is null" << endl;
//         } else {
//             state_machine_->TransitTo(st);
//         }
//     }

//     friend class StateMachineBase<StateIdEnumT, EventIdEnumT>;
// private:
//     const string st_name;
//     const StateIdEnumT st_id;
// };
// // template<typename StateIdEnumT, typename EventIdEnumT> StateMachineBase<StateIdEnumT, EventIdEnumT> StateBase<StateIdEnumT, EventIdEnumT>::state_machine_;
// template<typename StateIdEnumT, typename EventIdEnumT> shared_ptr<StateMachineBase<StateIdEnumT, EventIdEnumT> > StateBase<StateIdEnumT, EventIdEnumT>::state_machine_ = nullptr;
