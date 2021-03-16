#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

using namespace std;

class Serial {
private:
  string port_;

public:
  Serial() : fd_(-1) {}
  bool IsPortExists() {
    return !port_.empty() && access(port_.c_str(), F_OK) != -1;
  }
  bool Open(string port, int baud) {
    if (port.empty()) {
      cout << "port empty" << endl;
      return false;
    }

    fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd_ == -1) {
      if (errno == EINTR) {
        // recurse because it's a recoverable error
        return Open(port, baud);
      } else {
        perror("open");
        return false;
      }
    }

    // configure port
    struct termios options;
    if (tcgetattr(fd_, &options) == -1) {
      cout << "tcgetattr failed" << endl;
      return false;
    }
    // set up raw mode / no echo / binary
    options.c_cflag |= (tcflag_t)(CLOCAL | CREAD);
    options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL |
                                    ISIG | IEXTEN); //|ECHOPRT
    options.c_oflag &= (tcflag_t) ~(OPOST);
    options.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK);
#ifdef IUCLC
    options.c_iflag &= (tcflag_t)~IUCLC;
#endif
#ifdef PARMRK
    options.c_iflag &= (tcflag_t)~PARMRK;
#endif
    // set baud (just support common setting)
    switch (baud) {
    case 9600:
      ::cfsetispeed(&options, B9600);
      ::cfsetospeed(&options, B9600);
      break;
    case 19200:
      ::cfsetispeed(&options, B19200);
      ::cfsetospeed(&options, B19200);
      break;
    case 38400:
      ::cfsetispeed(&options, B38400);
      ::cfsetospeed(&options, B38400);
      break;
    case 57600:
      ::cfsetispeed(&options, B57600);
      ::cfsetospeed(&options, B57600);
      break;
    case 115200:
      ::cfsetispeed(&options, B115200);
      ::cfsetospeed(&options, B115200);
      break;
    default:
      cout << "not support baud " << baud << endl;
      return false;
    }

    // support 8N1 only
    options.c_cflag &= (tcflag_t)~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= (tcflag_t) ~(CSTOPB);
    options.c_iflag &= (tcflag_t) ~(INPCK | ISTRIP);
    options.c_cflag &= (tcflag_t) ~(PARENB | PARODD);
    // no flow control
    options.c_iflag &= (tcflag_t) ~(IXON | IXOFF | IXANY);
    // no rtscts
    options.c_cflag &= (unsigned long)~(CRTSCTS);

    // non-blocking read
    // (http://www.unixwiz.net/techtips/termios-vmin-vtime.html)
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    // apply the setting
    ::tcsetattr(fd_, TCSANOW, &options);

    // create a pipe for canceling read operation
    if (pipe(pipe_fd_) != 0) {
      perror("pipe");
      return false;
    }

    port_ = port;
    return true;
  }

  bool Close() {
    if (fd_ != -1) {
      int ret = ::close(fd_);
      if (ret == 0) {
        fd_ = -1;
        port_.clear();
        return true;
      } else {
        perror("close");
      }
    } else {
      cout << "already closed" << endl;
    }
    return false;
  }

  vector<char> Read(int timeout_ms) {
    // setup a select call to block for serial data or a timeout
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd_, &readfds);

    // for exiting from a blocking select call
    static const int PIPE_READ_FD = 0;
    FD_SET(pipe_fd_[PIPE_READ_FD], &readfds);

    int maxfd =
        max(fd_, pipe_fd_[PIPE_READ_FD]); /* maximum bit entry (fd) to test */
    vector<char> ret;

    struct timeval *ptv = NULL;
    if (timeout_ms != 0) {
      ptv = new struct timeval();
      ptv->tv_sec = timeout_ms / 1000;                        /* seconds */
      ptv->tv_usec = timeout_ms - 1000 * (timeout_ms / 1000); /* milliseconds */
    }
    /* set timeout value within input loop */

    static const int BUFFER_SIZE = 65536;
    if (select(maxfd + 1, &readfds, NULL, NULL, ptv) != -1) {
      if (FD_ISSET(pipe_fd_[PIPE_READ_FD], &readfds)) {
        cout << "select return for pipe_fd_" << endl;
        char tmp[BUFFER_SIZE];
        read(pipe_fd_[PIPE_READ_FD], tmp, BUFFER_SIZE); // clear pipe
      } else { // serial read buffer available
        ssize_t nbytes = 0;
        ssize_t total_bytes_read = 0;
        char buf[BUFFER_SIZE];
        while (nbytes = read(fd_, buf + total_bytes_read,
                             BUFFER_SIZE - total_bytes_read) > 0) {
          total_bytes_read += nbytes;
          ret.insert(ret.end(), buf, buf + total_bytes_read);
        }
        if (nbytes == -1) { // error
          perror("read");
        }
        if (nbytes == 0) { // eof
          if (IsPortExists()) {
            cout << "reach EOF" << endl;
          } else {
            cerr << "port not exists" << endl;
            ret.clear();
            Close();
            return ret;
          }
        }
      }
    } else {
      perror("select");
    }
    return ret;
  }

  bool StopRead() {
    static const int PIPE_WRITE_FD = 1;
    ssize_t nbytes = write(pipe_fd_[PIPE_WRITE_FD], "a", 1);
    if (nbytes != 1) {
      perror("write");
      return false;
    }
    return true;
  }

private:
  int fd_;
  int pipe_fd_[2];
};

Serial *serial;
bool running = true;

void SerialRead() {
  while (running) {
    vector<char> data = serial->Read(0);
    for (auto d : data) {
      cout << d;
    }
    cout << endl;
  }
}
#include <thread>
int main(int argc, char *argv[]) {
  // Tip:
  // use 'socat pty,raw,echo=0,link=/home/user/dev/ttyS20
  // pty,raw,echo=0,link=/home/user/dev/ttyS21' to create virtual serail ports
  // for testing
  serial = new Serial();
  if (!serial->Open(argv[1], 115200)) {
    cout << "Serial Open failed" << endl;
    return 1;
  }
  thread t1(SerialRead);
  char c;
  while (running) {
    cin >> c;
    switch (c) {
    case 'q':
      running = false;
      serial->StopRead();
      break;
    case 's': // stop read
      if (!serial->StopRead()) {
        cerr << "StopRead failed" << endl;
        return 1;
      }
    }
  }
  t1.join();
  return true;
}