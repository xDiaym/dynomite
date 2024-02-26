#include <chrono>
#include <string>
#include <format>
#include <iostream>

template <typename T>
inline void __attribute__((__always_inline__)) noopt(const T& t) {
  asm volatile("" : : "r,m"(t) : "memory");
}

class Benchmarker {
  std::string_view name;
  std::chrono::steady_clock::time_point start;
public:

  Benchmarker(std::string_view name_)
    : name(name_), start(std::chrono::steady_clock::now()) {}

  ~Benchmarker() {
    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << std::format("{}: {}.{}s", name, ms.count() / 1000, ms.count() % 1000) << std::endl;
  }
};

#define STRCAT(x, y) x##y
#define BENCHMARK(name) Benchmarker STRCAT(__bench, __LINE__)(name)