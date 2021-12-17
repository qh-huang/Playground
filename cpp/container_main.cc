#include <iostream>
#include <vector>

using namespace std;

int main (int argc, char* argv[]) {

    vector<int> vec;
    vec.push_back(0);

    // for (int i : vec) {
    //     cout << "pushing " << i+1 << endl;
    //     vec.push_back(i+1);
    // }

    for (int i=0; i<vec.size(); i++) {
        cout << "pushing " << i+1 << endl;
        vec.push_back(i+1);
    }

    cout << "hello world" << endl;
    return 0;
}