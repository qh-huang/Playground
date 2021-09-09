#include "looper.h"
#include <iostream>

using namespace std;

class Spinner : public Looper
{
public:
    Spinner(): Looper("spinner", 100) {}

    void SpinOnce() override {
        static int i = 0;
        cout << "spin once #" << i++ << endl;
    }

    bool WaitCondition() override {
        return true;
    }
};

int main(int argc, char *argv[]) {
  shared_ptr<Spinner> tn = make_shared<Spinner>();
  tn->Activate();

  int input;
  cin >> input;  

  return 0;
}