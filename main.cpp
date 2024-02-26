#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <ranges>
#include <iostream>
#include <functional>
#include <type_traits>
#include "bench.hpp"

constexpr int K_MAX = 1 << 24;


namespace dynamic {

class Animal {
public:
  virtual char speak() const = 0;
  virtual ~Animal() = default;
};

class Dog : public Animal {
public:
  int idx = 0;
  Dog(int i) : idx(i) {}
  char speak() const {
    return "barkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkb"[idx];
  }
};

class Cat : public Animal {
public:
  int idx = 0;
  Cat(int i) : idx(i) {}
  char speak() const {
    return "meowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowm"[idx];
  }
};

}

namespace static_ {

template <typename T>
concept NonVirtual = !std::is_polymorphic<T>::value;

struct Dog {
  int idx = 0;
  Dog(int i) : idx(i) {}
  char speak() const {
    return "barkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkbarkb"[idx];
  }
};

struct Cat {
  int idx = 0;
  Cat(int i) : idx(i) {}
  char speak() const {
    return "meowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowmeowm"[idx];
  }
};


template <typename T>
struct typeholder {
  typeholder() = default;
  using type = T;
};


struct VMT {
  void (*destroy)(uint8_t* self);
  char (*speak)(uint8_t* self);

  template <typename T>
  VMT(typeholder<T>) {
    destroy = [] (uint8_t* self) {
      reinterpret_cast<T*>(self)->~T();
    };
    // GENERATED
    speak = [] (uint8_t* self) {
      return reinterpret_cast<T*>(self)->speak();
    };
  }
};

~~
class Wrap {
private:
  uint8_t* data_;
  VMT* vmtp_;

public:
  template <NonVirtual T, typename ...Args>
  Wrap(typeholder<T>, Args&&... args) {
    data_ = reinterpret_cast<uint8_t*>(std::aligned_alloc(alignof(T), sizeof(T)));
    new (data_) T(std::forward<Args>(args)...);
    vmtp_ = new VMT(typeholder<T>());
  }

  // GENERATED
  char speak() const {
    return vmtp_->speak(data_);
  }

  Wrap(Wrap&& other) {
    data_ = std::exchange(other.data_, nullptr);
    vmtp_ = std::exchange(other.vmtp_, nullptr);
  }
  Wrap(const Wrap&) = delete; //{ std::cout << "Copied\n"; }

  Wrap& operator=(Wrap&& other) {
    data_ = std::exchange(other.data_, nullptr);
    vmtp_ = std::exchange(other.vmtp_, nullptr);
    return *this;
  }
  Wrap& operator=(const Wrap& ) = delete; //{ std::cout << "=Copied\n"; return *this; }

  ~Wrap() {
    if (vmtp_ && data_)
      vmtp_->destroy(data_);
    delete data_;
    delete vmtp_;
  }
};

}

void bench_dynamic() {
  std::vector<std::unique_ptr<dynamic::Animal>> v;
  srand(0);
  for (int _ : std::views::iota(0, K_MAX)) {
    if (rand() % 2)
      v.push_back(std::make_unique<dynamic::Dog>(rand() % 32));
    else
      v.push_back(std::make_unique<dynamic::Cat>(rand() % 32));
  }
  {
    BENCHMARK("dynamic");
    for (auto&& p : v)
      noopt(p->speak());
  }
}

void bench_static() {
  using namespace static_;
  std::vector<static_::Wrap> v;
  srand(0);
  for (int _ : std::views::iota(0, K_MAX)) {
    if (rand() % 2)
      v.push_back(Wrap(typeholder<Dog>(), rand() % 32));
    else
      v.push_back(Wrap(typeholder<Cat>(), rand() % 32));
  }
  {
    BENCHMARK("static");
    for (auto&& p : v)
      noopt(p.speak());
  }
}

int main() {
  bench_dynamic();
  bench_static();

  return 0;
}
