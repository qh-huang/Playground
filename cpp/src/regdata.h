#ifndef REGDATA_H__
#define REGDATA_H__

#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

using namespace std;

using Timestamp = uint64_t;
using Duration = uint64_t;

inline Timestamp GetCurrentTimestamp() { return 0; }

class TData
{
public:
    void UpdateTimestamp() {
//        lock_guard<mutex> lock(mtx);
        timestamp = GetCurrentTimestamp();
    }

    Timestamp GetTimestamp() {
//        lock_guard<mutex> lock(mtx);
        return timestamp;
    }

private:
    Timestamp timestamp;
//    mutex mtx;
};

template<typename T>
class TDataImpl : public TData
{
public:
    void SetInst(const T& t);
    T GetInst();

    TDataImpl(const T& t): t_(t)
    {
         UpdateTimestamp();
    }

    virtual ~TDataImpl() {}
private:
    T t_;
    static bool registered_;
};

unordered_map<string, shared_ptr<TData> > dashboard;

template<typename T>
bool SetTData(const string name, const T& t)
{
    dashboard[name] = make_shared<TDataImpl<T> >(t);
    return true;
}

template<typename T>
shared_ptr<T> GetTData(const string name)
{
    auto itor = dashboard.find(name);
    if (itor != dashboard.end()) {
        return itor->second;
    }
    return nullptr;
}



#endif // REGDATA_H__