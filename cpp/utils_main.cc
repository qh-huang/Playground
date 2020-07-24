#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

std::string& TrimLeft(std::string& str)
{
    auto it2 = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(str.begin(), it2);
    return str;
}

std::string& TrimRight(std::string& str)
{
    auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(it1.base(), str.end());
    return str;
}

std::string& Trim(std::string& str) { return TrimLeft(TrimRight(str)); }

std::string TrimAndCopy(std::string const& str)
{
    auto s = str;
    return TrimLeft(TrimRight(s));
}

std::string JoinPath(const std::string& heading_path,
                     const std::string& tail_path) {
#ifdef _WIN32
  static constexpr char separator = '\\';
#else  // Linux, OSX
  static constexpr char separator = '/';
#endif
  if ((!tail_path.empty() && tail_path[0] == separator) ||
      heading_path.empty()) {
    return tail_path;
  } else if (heading_path[heading_path.size() - 1] == separator) {
    return heading_path + tail_path;
  } else {
    return heading_path + std::string(&separator, 1) + tail_path;
  }
}

template <typename Out>
void split_(const std::string& s, char delim, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
    *result++ = item;
  }
}

std::vector<std::string> Split(const std::string& s, char delim) {
  std::vector<std::string> elems;
  split_(s, delim, std::back_inserter(elems));
  return elems;
}

std::string ExtractDirectory(const std::string& file_path) {
  std::string dir_path;
  size_t found = file_path.find_last_of("/\\");
  if (found != std::string::npos) dir_path = file_path.substr(0, found);
  return dir_path;
}

std::string ExtractFileName(const std::string& file_path) {
  std::string file_name;
  size_t found = file_path.find_first_of("/\\");
  if (found != std::string::npos) file_name = file_path.substr(found + 1);
  return file_name;
}

bool FileExists(const std::string& name)
{
    std::ifstream ifs(name);
    bool good = ifs.good();
    if (!good)
    {
        std::cerr << name << " does not exists";
    }
    return good;
}

bool DirExists(const std::string& name)
{
    struct stat file_info;
    if (stat(name.c_str(), &file_info) != 0)
    {
        return false;
    }
    return (file_info.st_mode & S_IFDIR) != 0;
}

bool CreateDirIfNotExists(const std::string& name)
{
    if (DirExists(name) == false)
    {
        int status = mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        return status == 0;
    }
    return true;
}

std::vector<std::string> FileToLines(const std::string& file_path) {
  std::vector<std::string> lines;
  std::ifstream file(file_path.c_str(), std::ios::in);
  if (file.good()) {
    std::string str;
    while (std::getline(file, str)) {
      lines.push_back(str);
    }
  }
  return lines;
}

// TODO: this function sometimes return too early if the command has multiple response
std::string Exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

// return a current time as string
// as format like 2020-01-23_01:23:45:678
std::string GetCurrentTime()
{
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();

    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

    time_t tt = std::chrono::system_clock::to_time_t(now);

    tm t = *gmtime(&tt);

    char buffer[50];
    sprintf(buffer, "%04u-%02u-%02u_%02u:%02u:%02u:%03u", t.tm_year + 1900,
            t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
            static_cast<unsigned>(tp / std::chrono::milliseconds(1)));
    std::string ret(buffer);
    return ret;
}

// Timer for time measurement
class Timer
{
public:
    Timer() {}
    inline void Start() { clock_gettime(CLOCK_MONOTONIC, &time1); }
    inline double GetElapsed_ms() { return GetElapsed_ns() / 1000000.0; }
    inline double GetElapsed_us() { return GetElapsed_ns() / 1000.0; }
    inline double GetElapsed_ns()
    {
        clock_gettime(CLOCK_MONOTONIC, &time2);
        return (
            (1000000000.0 * static_cast<double>(time2.tv_sec - time1.tv_sec)) +
            static_cast<double>(time2.tv_nsec - time1.tv_nsec));
    }

private:
    struct timespec time1, time2;
};

template <typename Duration = std::chrono::microseconds, typename F,
          typename... Args>
typename Duration::rep Profile(F&& fun, Args&&... args) {
  const auto beg = std::chrono::high_resolution_clock::now();
  std::forward<F>(fun)(std::forward<Args>(args)...);
  const auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<Duration>(end - beg).count();
}

void foo(std::size_t N) {
  long double x{1.234e5};
  for (std::size_t k = 0; k < N; k++) {
    x += std::sqrt(x);
  }
}

int main(int argc, char** argv) {
  // JoinPath
  const std::string s1("/tmp");
  const std::string s2("temp_file");
  std::cout << JoinPath(s1, s2) << std::endl;

  // Split
  const std::string s3("apple::banana:cat:::dog::elephant:frog");
  std::vector<std::string> strs = Split(s3, ':');
  for (const auto& str : strs) {
    std::cout << str << std::endl;
  }

  // Profile
  std::size_t N{1000000};

  // profile in default mode (microseconds)
  std::cout << "foo(1E6) takes " << Profile(foo, N) << " microseconds"
            << std::endl;

  // profile in custom mode (e.g, milliseconds)
  std::cout << "foo(1E6) takes " << Profile<std::chrono::milliseconds>(foo, N)
            << " milliseconds" << std::endl;

  // To create an average of `M` runs we can create a vector to hold
  // `M` values of the type used by the clock representation, fill
  // them with the samples, and take the average
  std::size_t M{100};
  std::vector<typename std::chrono::milliseconds::rep> samples(M);
  for (auto& sample : samples) {
    sample = Profile(foo, N);
  }
  auto avg = std::accumulate(samples.begin(), samples.end(), 0) /
             static_cast<long double>(M);
  std::cout << "average of " << M << " runs: " << avg << " microseconds"
            << std::endl;
  return 0;
}
