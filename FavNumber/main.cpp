#include <iostream>

int main() {
  std::cout << "Enter your favorite number between 1 and 100: " << std::endl;
  int num = 0;
  std::cin >> num;

  if (num < -1 || num > 100) {
    std::cout << "Sorry, you entered an invalid number. Please try again." << std::endl;
  }
  else {
    std::cout << "Wow! " << num << " is my favorite number too!" << std::endl;
  }
  return 0;
}