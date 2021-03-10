#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

using namespace std;

void foo()
{
    // simulate expensive operation
    this_thread::sleep_for(chrono::seconds(1));
}
 
void bar()
{
    // simulate expensive operation
    while (true)
        this_thread::sleep_for(chrono::seconds(1));
}
 
int main()
{
    cout << "starting first helper...\n";
    thread helper1(foo);
 
    cout << "starting second helper...\n";
    shared_ptr<thread> helper2 = make_shared<thread>(bar);
 
    cout << "waiting for helpers to finish..." << endl;
    helper1.join();

    // helper2->join();
    try {
        helper2.reset();
    } catch (exception e) {
        cout << e.what() << endl;
    }
 
    cout << "done!\n";
}