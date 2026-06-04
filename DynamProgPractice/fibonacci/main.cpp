#include <iostream>
//need to include unordered map for memoized code
#include <unordered_map>

// //the following is the brute force implementation of the fibonacci calculation:
// int fib(int n) {
//   if (n <= 2) return 1;
//   return (fib(n-1) + fib(n-2));
// }

/*
  now lets implement a memoized fibonacci calcuation:
    this one will store previous values to make the lookup tree
    much smaller.
    Time: O(n)
    Space: O(n)
*/
typedef unsigned long long ull;

//memoized recursize func:
//make sure to pass by ref the memo
ull fibHelper(int n, std::unordered_map<int, ull>& memo) {
  //check if n is already in the memo map:
  //this is done via the .contains(), which is a read-only
  //method. otherwise, simply using something like if (memo[n])
  //would populate the map if n does not exist
  if (memo.contains(n)) { 
    return memo[n];
  }

  //base case:
  if (n <= 2) return 1;

  //calculate, store in memo, and return:
  memo[n] = fibHelper(n-1, memo) + fibHelper(n-2, memo);
  return memo[n];
}

//make a wrapper function to keep main() clean
ull fib(int n) {
  std::unordered_map<int, ull> memo;
  return fibHelper(n, memo);
}

int main () {

  std::cout << "fib(6) = " << fib(6) << std::endl;
  std::cout << "fib(7) = " << fib(7) << std::endl;
  std::cout << "fib(90) = " << fib(90) << std::endl;
  //anything larger than fib(93) will overflow the unsigned long long.
  //need either a custom BigINT or something to holder larger values.

  return 0;
}