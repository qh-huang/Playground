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
    explicit TimerThread(const function<void()>& task): repeat_(false), running_(false), task_(task) {}
    ~TimerThread() { StopTimer(); }

    TimerThread(const TimerThread&) = delete;
    TimerThread(TimerThread&&) = delete;
    TimerThread& operator=(const TimerThread&) = delete;
    TimerThread& operator=(TimerThread&&) = delete;

public:
    bool StartTimer(uint64_t interval_ms, bool repeat = true) {
        if (thread_.get_id() == this_thread::get_id()) return false;
        if (running_) return false;
        repeat_ = repeat;
        running_ = true;
        thread_ = thread([this, interval_ms]() {
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
        return true;
    }
    bool IsRunning() const { return running_; }

private:
    atomic<bool> repeat_;
    atomic<bool> running_;
    function<void()> task_;
    mutex mutex_;
    condition_variable cv_timer_;
    thread thread_;
};

using TimerId = uint32_t;


// static int g_cnt = 0;
int main(int argc, char* argv[])
{
    shared_ptr<TimerThread> tt0 = make_shared<TimerThread>([&]{ cout << "hello" << endl; });
    tt0->StartTimer(1000, true);

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(3100));
        tt0->StopTimer();
        break;
    }
    return 0;
}



