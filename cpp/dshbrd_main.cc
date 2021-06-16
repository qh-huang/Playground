#include "dashboard.h"

class TestPublisher : public MsgBus::Publisher, public Looper
{
public:
    TestPublisher(): Looper("test_pub", 500) {
        Looper::Activate();
    }

    // Looper::SpinOnce()
    void SpinOnce() override 
    {
        static int boot_cnt = 1;
        SysInfoMsgPtr sys_info = make_shared<SysInfoMsg>();
        sys_info->data.boot_cnt_loaded = true;
        sys_info->data.boot_cnt = boot_cnt++;
        Publish(sys_info);
    }

    // Looper::WaitCondition()
    bool WaitCondition() override 
    {
        return true; // keep spinning
    }

};

int main(int argc, char *argv[]) 
{
    Dashboard db;

    db.Activate();
    
    // no data yet, should NOT get any data now
    SysInfo sysinfo;
    if (db.GetData<SysInfo>(DataType::SYS_INFO, sysinfo)) {
        Print(sysinfo);
    } else {
        cout << "cannot get SysInfo" << endl;
    }

    this_thread::sleep_for(chrono::milliseconds(3000));

    TestPublisher test_pub;

    while (true) {
        SysInfo sys_info;
        if (db.GetData<SysInfo>(DataType::SYS_INFO, sys_info)) {
            Print(sys_info);
        } else {
            cerr << "cannot get SysInfo" << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
    }



    return 0;
}