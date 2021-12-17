#include <iostream>

using namespace std;

class Base
{
public:
    Base(string name): base_name_(name + "_base") {
        cout << hex << this << " Base::Base()" << ", base_name: " << base_name_ << endl;
    }
private:
    const string base_name_;
};

class DerivedA : public Base
{
public:
    DerivedA(string name): Base(name + "_derived_a") {
        cout << hex << this << " DerivedA::DerivedA()" << endl;
    }
};

class Test : public Base, public DerivedA// , public Base
{
public:
    Test(): Base("test"), DerivedA("test") {
        cout << hex << this << endl;
    }
};

class Test2 : public DerivedA// , public Base
{
public:
    Test2(): DerivedA("test2") {
        cout << hex << this << endl;
    }
};


int main(int argc, char* argv[])
{
    Test test;
    Test2 test2;
    return 0;
}