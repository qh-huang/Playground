#pragma once

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

class Looper
{
public:
    Looper(string name, uint32_t spin_interval_ms = 0): 
        looper_name_(name + "_looper"), 
        thread_run_(false), 
        spin_interval_ms_(spin_interval_ms) {}

    virtual ~Looper() { Deactivate(); }

    bool IsActive() { return (looper_thread_)? true : false; }

    void Activate()
    {
        if (looper_thread_)
        {
            cerr << "[" << looper_name_ << "] is already running" << endl;
            return;
        }
        thread_run_ = true;
        looper_thread_ = make_shared<thread>(&Looper::ThreadLoop, this);
    }

    void Deactivate()
    {
        if (!IsActive()) {
            return;
        }

        thread_run_ = false;
        if (looper_thread_)
        {
            cv_.notify_one();
            cout << "[" << looper_name_ << "] wating for thread to join" << endl;
            looper_thread_->join();
            cout << "[" << looper_name_ << "] thread joined" << endl;
            looper_thread_.reset();
        }
    }

    void SetSpinIntervalMs(uint32_t spin_interval_ms) { spin_interval_ms_ = spin_interval_ms; }

    virtual void SpinOnce() = 0;
    virtual bool WaitCondition() = 0;

protected:
    void Notify()
    {
        cv_.notify_one();
    }

private:
    void ThreadLoop()
    {
        cout << "[" << looper_name_ << "] start looping" << endl;
        while (thread_run_)
        {
            unique_lock<mutex> lk(mtx_cv_);
            // cout << "thread waits for notify";
            cv_.wait(lk, [this] {
                return !thread_run_ || WaitCondition();
            });
            // cout << "thread was notified";
            lk.unlock();

            // thread_run_ might be toggled by another thread
            if (!thread_run_)
                break;

            SpinOnce();

            if (spin_interval_ms_ != 0) {
                this_thread::sleep_for(chrono::milliseconds(spin_interval_ms_));
            }
        }
        cout << "[" << looper_name_ << "] stop looping" << endl;
    }

    shared_ptr<thread> looper_thread_;
    bool thread_run_;
    condition_variable cv_;
    mutex mtx_cv_;

    const string looper_name_;
    uint32_t spin_interval_ms_;
};

