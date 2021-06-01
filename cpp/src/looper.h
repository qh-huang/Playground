#pragma once

#include <condition_variable>
#include <string>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

class Looper
{
public:
    Looper(string name): looper_name_(name), thread_run_(false) {}
    virtual ~Looper() { Deactivate(); }

    void Activate() {
        if (looper_thread_) {
            cerr << "lopper " << looper_name_ << " is already running" << endl;
            return;
        }
        thread_run_ = true;
        looper_thread_ = make_shared<thread>(&Looper::ThreadLoop, this);
    }

    void Deactivate() {
        thread_run_ = false;
        if (looper_thread_) {
            cv_.notify_one();
            cout << "[" << looper_name_ << "] wating for thread to join" << endl;
            looper_thread_->join();
            cout << "[" << looper_name_ << "] thread joined" << endl;
            looper_thread_.reset();
        }
    }

    virtual void SpinOnce() = 0;
    virtual bool WaitCondition() = 0;
protected:
    void Notify()
    {
        cv_.notify_one();
    }
private:
    void ThreadLoop() {
        cout << looper_name_ << " start looping..." << endl;
        while(thread_run_) {
            unique_lock<mutex> lk(mtx_cv_);
            cout << "thread waits for notify" << endl;
            cv_.wait(lk, [this] {
                return !thread_run_ || WaitCondition();
            });
            cout << "thread was notified" << endl;
            lk.unlock();

            // thread_run_ might be toggled by another thread
            if (!thread_run_)
                break;

            SpinOnce();
        }
        cout << looper_name_ << " stop looping" << endl;
  }

    shared_ptr<thread> looper_thread_;
    bool thread_run_;
    condition_variable cv_;
    mutex mtx_cv_;

    const string looper_name_;
};