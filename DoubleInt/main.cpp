#include <iostream>

int main(){
  std::cout << "Enter an integer: ";
  int x{};
  std::cin >> x;
  std::cout << "Double that numer is: " << x * 2 << std::endl;
  return 0;
}