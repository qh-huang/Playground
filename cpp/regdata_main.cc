#include "src/regdata.h"

class DataA : public TData {
public:
  int a;
};

int main(int argc, char *argv[]) {
  DataA a1;
  // shared_ptr<DataA> a1 = make_shared<DataA>();
  a1.a = 1;
  // a1->a = 1;

  SetTData<DataA>(string("A"), a1);

  // ................

  // DataA a2 = DataMap::Instance().Get("A");

  return 0;
}