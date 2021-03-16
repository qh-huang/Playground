#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

std::string Exec(const char *cmd, int timeout_ms) {
  static const int BUFFER_SIZE = 1024;
  std::array<char, BUFFER_SIZE> buffer;
  std::string result;

  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    std::cerr << "popen() failed!" << std::endl;
    return result;
  }

  struct timeval timeout;
  timeout.tv_sec = timeout_ms / 1000;           /* seconds */
  timeout.tv_usec = 1000 * (timeout_ms % 1000); /* microseconds */

  int fd = fileno(pipe.get());
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);

  ssize_t nbytes = 0;
  switch (select(fd + 1, &readfds, NULL, NULL, &timeout)) {
  case 0:
    // timeout expired
    std::cerr << "timeout expired" << std::endl;
    break;
  case -1:
    if ((errno == EINTR) || (errno == EAGAIN)) {
      errno = 0;
      break;
    } else {
      std::cerr << "select() failed" << std::endl;
      exit(-1);
    }
  case 1: /* We have input */
    nbytes = read(fd, buffer.data(), BUFFER_SIZE);
    result.append(buffer.data(), nbytes);
    break;
  default:
    std::cerr << "input on more than one file descriptor, should never happen"
              << std::endl;
  }
  return result;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "no arguments" << std::endl;
    return 1;
  }

  std::cout << Exec(argv[1], 1000) << std::endl;

  return 0;
}