#include <iostream>

#define LOG(x) std::cout << x << std::endl;

int main(){
  int var = 8; //define an integer called var
  int* ptr = &var; //assign the memory address of var (via the & symbol) to the pointer ptr

  //what if I want to change that data?
  *ptr = 10; //dereference the ptr via the * symbol, then change the value at that memory address (var)
  LOG(var)
  std::cin.get();
}