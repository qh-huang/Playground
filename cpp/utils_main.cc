#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

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