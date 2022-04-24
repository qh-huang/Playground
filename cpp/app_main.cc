#include <algorithm>
#include <iostream>
#include <memory>
#include <array>

#define DISPLAY_FUNC_NAME                                                      \
  do {                                                                         \
    std::cout << __func__ << std::endl;                                        \
  } while (0);

class Base {
public:
  Base() { DISPLAY_FUNC_NAME }
  virtual ~Base() { DISPLAY_FUNC_NAME }
};

class Derive : public Base {
public:
  Derive() { DISPLAY_FUNC_NAME }
  virtual ~Derive() { DISPLAY_FUNC_NAME }
};

int main(int argc, char **argv) {
  // bad idea: avoid using raw pointer
  {
    Derive *p = new Derive();
    delete (p);
  }
  // good idea: use unique_ptr/shared_ptr instead
  {
    // std::unique_ptr<Derive> p = std::make_unique<Derive>(); // C++ 14
    std::unique_ptr<Derive> p = std::unique_ptr<Derive>(new Derive()); // C++ 11
  }

  // replace old-style array and looping
  {
    // instead of int arr[10] = {...};
    std::array<int, 10> arr{1, 1, 2, 3, 5, 8, 13, 21, 34, 55};

    // instead of for (int i=0; i<arr.size(); i++) { ... }
    std::for_each(arr.begin(), arr.end(),
                  [&](const int i) -> void { std::cout << i << ' '; });
    std::cout
        << '\n'; // std::endl implies a std::flush, only use it when needed
  }

  return 0;
}
