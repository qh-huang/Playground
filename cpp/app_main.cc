#include <iostream>

#define DISPLAY_FUNC_NAME               \
  do {                                  \
    std::cout << __func__ << std::endl; \
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

int main(int argc, char** argv) {
  {
    Derive* p = new Derive();
    delete(p);
  }
  return 0;
}
