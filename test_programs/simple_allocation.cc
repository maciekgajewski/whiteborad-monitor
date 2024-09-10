#include <fmt/core.h>

#include <numeric>
#include <string>

void populate(int *data, int size) {
  for (int i = 0; i < size; i++) {
    data[i] = i;
  }
}

int main(int argc, char **argv) {

  if (argc < 2) {
    fmt::println("Argument required");
    return -1;
  }
  fmt::println("Argument: {}", argv[1]);

  int size = std::stoi(argv[1]);

  int *data = new int[size];
  populate(data, size);
  int sum = std::accumulate(data, data + size, 0);
  delete[] data;
  return sum;
}