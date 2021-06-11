#pragma once

#include "looper.h"

#include <queue>

using namespace std;

template<typename T>
class Dispatcher : public Looper
{
public:
    Dispatcher(string name) : Looper(name + "_dispatcher") {}
    virtual ~Dispatcher() = default;

    void Dispatch(const T& t) {
        lock_guard<mutex> lock(mtx_queue_);
        queue_.push();
        Looper::Notify();
    }

    // return true if successfully processed
    virtual bool Process(T& t) = 0;

    // Looper::SpinOnce
    void SpinOnce() override {
        lock_guard<mutex> lock(mtx_queue_);

        // proceess 1 per spin_once to allow Dispatch() push new Data
        if (!queue_.empty()) {
            T& t = queue_.front();
            if (Process(t)) {
                queue_.pop();
            }
        }        
    }

    // Looper::WaitCondition
    bool WaitCondition() override 
    {
        return !queue.empty();
    }

private:
    mutex mtx_queue_;
    queue<T> queue_;
};