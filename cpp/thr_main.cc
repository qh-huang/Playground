#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace std;

void foo() {
  // simulate expensive operation
  this_thread::sleep_for(chrono::seconds(1));
  cout << "exiting foo" << endl;
}

void bar() {
  // simulate expensive operation
  while (true)
    this_thread::sleep_for(chrono::seconds(1));
}

void detach_me() {
  this_thread::sleep_for(chrono::milliseconds(10));
  cout << "exiting detach_me" << endl;
}

int main() {
  cout << "starting first helper...\n";
  thread helper1(foo);

  cout << "starting second helper...\n";
  shared_ptr<thread> helper2 = make_shared<thread>(bar);

  cout << "starting third helper...\n";
  thread helper3(detach_me);
  helper3.detach();

  cout << "waiting for helpers to finish..." << endl;
  helper1.join();

  // helper2->join();
  try {
    helper2->detach();
    helper2.reset();
  } catch (exception e) {
    cout << e.what() << endl;
  }

  cout << "done!\n";
}