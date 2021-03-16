#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

using namespace std;

using Timestamp = uint64_t;

class MsgBase {
public:
  MsgBase(){};
  virtual ~MsgBase(){}; // need one virtual method for dynamic_cast
};

template <typename T> class Msg : public MsgBase {
  // public:
  //     Msg(const T& t) : t_(t) {}
  //     Msg() : t_(T()) {}

  //     T t_;
};

unordered_map<string, shared_ptr<MsgBase>> dashboard;

template <typename T> void SetMsg(string name, const T &t) {
  // shared_ptr<Msg<T> > pT = make_shared<Msg<T>>(t);
  shared_ptr<T> pT = make_shared<T>(t);
  dashboard[name] = dynamic_pointer_cast<MsgBase>(pT);
}

template <typename T> void SetMsgPtr(string name, shared_ptr<T> pT) {
  dashboard[name] = dynamic_pointer_cast<MsgBase>(pT);
}

template <typename T> T GetMsg(string name) {
  auto itor = dashboard.find(name);
  if (itor != dashboard.end()) {
    shared_ptr<MsgBase> pMsgBase = itor->second;
    // shared_ptr<Msg<T> > pMsg = dynamic_pointer_cast<Msg<T> >(pMsgBase);
    // if (pMsg) return pMsg->t_;
    shared_ptr<T> pMsg = dynamic_pointer_cast<T>(pMsgBase);
    return *pMsg;
  }
  return T();
}

template <typename T> shared_ptr<T> GetMsgPtr(string name) {
  auto itor = dashboard.find(name);
  if (itor != dashboard.end()) {
    return dynamic_pointer_cast<T>(itor->second);
  }
  return nullptr;
}

class DataA : public MsgBase {
public:
  int a;
};

class DataS : public Msg<DataS> {
public:
  string s;
};

int main(int argc, char *argv[]) {
  DataA da1;
  da1.a = 123;
  SetMsg("a", da1);

  DataA da2 = GetMsg<DataA>("a");
  cout << da2.a << endl;

  shared_ptr<DataA> da3 = make_shared<DataA>();
  da3->a = 1234;
  SetMsgPtr("a", da3);

  // DataA da4 = GetMsg<DataA>("a");
  // cout << da4.a << endl;
  cout << GetMsgPtr<DataA>("a")->a << endl;

  DataS ds1;
  ds1.s = "hello";
  SetMsg("s", ds1);

  DataS ds2 = GetMsg<DataS>("s");
  cout << ds2.s << endl;

  return 0;
}