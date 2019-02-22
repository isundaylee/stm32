#include <iostream>

#include <libsigrokcxx/libsigrokcxx.hpp>

int main(int, char*[]) {
  auto context = sigrok::Context::create();

  return 0;
}
