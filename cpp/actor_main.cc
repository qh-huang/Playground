#include <iostream>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

class Actor
{
public:
  enum class State
  {
    STOPPED,
    STARTED
  };
  Actor()
  {
    state_ = State::STOPPED;
    thread_ = nullptr;
    activate_ = false;
  }
  virtual bool Start()
  {
    if (thread_)
    {
      std::cerr << "Cannot start: thread_ is not cleared yet" << std::endl;
      return false;
    }
    activate_ = true;
    thread_ = std::make_shared<std::thread>(&Actor::MainLoop, this);
    state_ = State::STARTED;
    return true;
  }
  virtual void Stop()
  {
    activate_ = false;
    if (thread_)
    {
      thread_->join();
      thread_.reset();
    }
    state_ = State::STOPPED;
  }

protected:
  virtual void MainLoop()
  {
    while (activate_)
    {
      // Note:
      // Never block this thread in this while scope
      std::cout << __func__ << ": Do Something..." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << __func__ << ": terminating thread(" << thread_->get_id() << ")" << std::endl;
  }

  std::shared_ptr<std::thread> thread_;
  bool activate_;
  State state_;
};

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <functional>

template <typename Out>
void split_(const std::string& s, char delim, Out result)
{
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim))
  {
    *result++ = item;
  }
}

std::vector<std::string> Split(const std::string& s, char delim)
{
  std::vector<std::string> elems;
  split_(s, delim, std::back_inserter(elems));
  return elems;
}

class CliAgent : public Actor
{
public:
  bool Start() override
  {
    if (pipe(parent_to_child_) != 0 || pipe(child_to_parent_) != 0)
    {
      std::cerr << __func__ << ": Create pipe failed" << std::endl;
      return false;
    }
    return Actor::Start();
  }

  void MainLoop()
  {
    if (close(parent_to_child_[READ_FD]) != 0 || close(child_to_parent_[WRITE_FD]) != 0)
    {
      std::cerr << __func__ << ": close file descripter failed" << std::endl;
      return;
    }
    char buffer[BUFFER_SIZE + 1];

    fd_set readfds;
    struct timeval timeout;

    timeout.tv_sec = 0;     /* Seconds */
    timeout.tv_usec = 1000; /* Microseconds */

    FD_ZERO(&readfds);
    FD_SET(child_to_parent_[READ_FD], &readfds);

    std::string data_read_from_child;

    ssize_t read_bytes;
    while (activate_)
    {
      switch (select(1 + child_to_parent_[READ_FD], &readfds, (fd_set*)NULL, (fd_set*)NULL, &timeout))
      {
        case 0:  // timeout
          std::cerr << __func__ << ": timeout" << std::endl;
          timeout.tv_sec = 2;  /* Seconds */
          timeout.tv_usec = 0; /* Microseconds */
          break;
        case -1:
          if ((errno == EINTR) || (errno == EAGAIN))
          {
            errno = 0;
            break;
          }
          else
          {
            std::cerr << __func__ << ": select() failed" << std::endl;
            exit(-1);
          }
        case 1: /* We have input */
          read_bytes = read(child_to_parent_[READ_FD], buffer, BUFFER_SIZE);
          data_read_from_child.append(buffer, read_bytes);
          std::cerr << __func__ << ": read " << read_bytes << " bytes; current result is [" << data_read_from_child
                    << "]" << std::endl;
          timeout.tv_sec = 0;     /* Seconds */
          timeout.tv_usec = 1000; /* Microseconds */
          break;

        default:
          std::cerr << "input on more than one file descriptor" << std::endl;
          exit(-1);
      }
    }
    std::cout << __func__ << ": terminating thread(" << thread_->get_id() << ")" << std::endl;
  }

  void Exec(std::string cmd)
  {
    if (dup2(parent_to_child_[READ_FD], STDIN_FILENO) == -1 || dup2(child_to_parent_[WRITE_FD], STDOUT_FILENO) == -1 ||
        dup2(child_to_parent_[WRITE_FD], STDERR_FILENO) == -1)
    {
      std::cerr << __func__ << ": dup2() failed" << std::endl;
      exit(-1);
    }

    std::vector<std::string> input = Split(cmd, ' ');

    std::vector<char*> args;
    args.reserve(input.size() + 1);
    for (std::string s : input)
    {
      args.push_back(const_cast<char*>(s.c_str()));
    }
    args.push_back(nullptr);  // needed to terminate the args list
    execvp(args[0], args.data());
  }

private:
  static constexpr int READ_FD = 0;
  static constexpr int WRITE_FD = 1;
  static constexpr int BUFFER_SIZE = 100;
  int parent_to_child_[2];
  int child_to_parent_[2];
};

using namespace std;
enum PIPE_FILE_DESCRIPTERS
{
  READ_FD = 0,
  WRITE_FD = 1
};

enum CONSTANTS
{
  BUFFER_SIZE = 100
};
int main(int argc, char** argv)
{
#if 0
  Actor actor;
  actor.Start();

  std::this_thread::sleep_for(std::chrono::seconds(10));
  actor.Stop();
// #endif
  CliAgent ca;
  ca.Start();

  std::this_thread::sleep_for(std::chrono::seconds(3));
  ca.Exec("ls -al");

  std::this_thread::sleep_for(std::chrono::seconds(3));

  ca.Stop();
#endif
  int parentToChild[2];
  int childToParent[2];
  pid_t pid;
  string dataReadFromChild;
  char buffer[BUFFER_SIZE + 1];
  ssize_t readResult;
  int status;

  assert(0 == pipe(parentToChild));
  assert(0 == pipe(childToParent));

  fcntl(childToParent[0], F_SETFL, O_NONBLOCK);

  switch (pid = fork())
  {
    case -1:
      printf("Fork failed");
      exit(-1);

    case 0: /* Child */
      assert(-1 != dup2(parentToChild[READ_FD], STDIN_FILENO));
      assert(-1 != dup2(childToParent[WRITE_FD], STDOUT_FILENO));
      assert(-1 != dup2(childToParent[WRITE_FD], STDERR_FILENO));
      assert(0 == close(parentToChild[WRITE_FD]));
      assert(0 == close(childToParent[READ_FD]));

      /*     file, arg0, arg1,  arg2 */
      execlp("ping", "ping", "8.8.8.8", NULL);

      printf("This line should never be reached!!!");
      exit(-1);

    default: /* Parent */
      cout << "Child " << pid << " process running..." << endl;

      assert(0 == close(parentToChild[READ_FD]));
      assert(0 == close(childToParent[WRITE_FD]));

      while (true)
      {
        switch (readResult = read(childToParent[READ_FD], buffer, BUFFER_SIZE))
        {
          case 0: /* End-of-File, or non-blocking read. */
            cout << "End of file reached..." << endl
                 << "Data received was (" << dataReadFromChild.size() << "): " << endl
                 << dataReadFromChild << endl;

            assert(pid == waitpid(pid, &status, 0));

            cout << endl << "Child exit staus is:  " << WEXITSTATUS(status) << endl << endl;

            exit(0);

          case -1:
            if ((errno == EINTR) || (errno == EAGAIN))
            {
              errno = 0;
              std::cerr << __func__ << ": readResult is -1" << std::endl;
              std::this_thread::sleep_for(std::chrono::seconds(3));
              break;
            }
            else
            {
              printf("read() failed");
              exit(-1);
            }

          default:
            dataReadFromChild.append(buffer, readResult);
            // std::cout << dataReadFromChild << std::endl << std::flush;
            std::cout << dataReadFromChild << std::endl;
            break;
        }
      } /* while (true) */
  }     /* switch (pid = fork())*/

  return 0;
}