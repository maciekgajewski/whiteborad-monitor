#include <vector>

int main(int, char **) {
  std::vector<int> v;
  //__builtin_trap();
  asm("int3; nop");
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
}