#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>

#include <iostream>

using namespace std;

class TimerThread
{
public:
    TimerThread(): repeat_(false), running_(false) {}
    ~TimerThread() { StopTimer(); }

    TimerThread(const TimerThread&) = delete;
    TimerThread(TimerThread&&) = delete;
    TimerThread& operator=(const TimerThread&) = delete;
    TimerThread& operator=(TimerThread&&) = delete;

public:
    bool SetTimerTask(const function<void()>& task) {
        if (thread_.get_id() == this_thread::get_id()) return false;
        if (running_) return false;
        task_ = task;
        return true;
    }
    void SetRepeat(bool repeat) { repeat_ = repeat; }
    bool StartTimer(uint64_t interval_ms) {
        if (thread_.get_id() == this_thread::get_id()) return false;
        if (running_) return false;
        running_ = true;
        thread_ = std::thread([this, interval_ms]() {
            while (running_)
            {
                {
                    unique_lock<mutex> lock(mutex_);
                    cv_timer_.wait_for(lock, chrono::milliseconds(interval_ms), [this] {
                        return !running_;
                    });
                }

                if (!running_)
                {
                    return;
                }

                task_();

                if (!repeat_) {
                    return;
                }
            } // end while
        }); // end thread_
        return true;
    }

    bool StopTimer() {
        if (thread_.get_id() == this_thread::get_id()) return false;
        running_ = false;
        cv_timer_.notify_one();
        if (thread_.joinable()) {
            thread_.join();
        }
        cout << "timer stopped" << endl;
        return true;
    }
    bool IsRunning() const { return running_; }

private:
    std::function<void()> task_;
    std::atomic<bool> running_;
    std::atomic<bool> repeat_;
    std::mutex mutex_;
    std::condition_variable cv_timer_;
    std::thread thread_;
};

using TimerId = uint32_t;

namespace FutureTask {

    unordered_map<TimerId, shared_ptr<TimerThread> > g_timer_threads;

    // return 0 if failed
    TimerId Create(uint32_t delay_ms, bool repeat, const function<void()>& f) {
        static TimerId timer_id = 1;
        g_timer_threads[timer_id] = make_shared<TimerThread>();
        if (!g_timer_threads[timer_id]->SetTimerTask(f)) return 0;
        g_timer_threads[timer_id]->SetRepeat(repeat);
        g_timer_threads[timer_id]->StartTimer(delay_ms);
        return timer_id++;
    }

    bool Stop(TimerId timer_id) {
        if (g_timer_threads.find(timer_id) == g_timer_threads.end()) return false;
        return g_timer_threads.at(timer_id)->StopTimer();
    }

    bool Destroy(TimerId timer_id) {
        // if (!Stop(timer_id)) return false;
        cout << "destroying timer " << timer_id << endl;
        return g_timer_threads.erase(timer_id) == 1; // 1: successfully erased; 0: failed to erase
    }
}


int main(int argc, char* argv[])
{
    TimerId timer_id = FutureTask::Create(1000, true, [&]{ cout << "hello world" << endl; });

    this_thread::sleep_for(chrono::milliseconds(5500));

    if (!FutureTask::Destroy(timer_id)) {
        cerr << "failed to destroy timer" << endl;
    }

    // while (true) this_thread::yield();
    while (true) this_thread::sleep_for(chrono::milliseconds(10000));

    return 0;
}



