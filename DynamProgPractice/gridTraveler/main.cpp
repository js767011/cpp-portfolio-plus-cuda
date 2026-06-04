/* Problem:
  Say that you are a traveler on a 2D grid.
  You begin in the top-left corner, and your goal is to travel to the bottom-right corner.
  You may only move down or right.

  In how many ways can you travel to the goal on a grid with dimensions m * n?
*/
#include <iostream>

//need the relevant includes for unordered_map and string
#include <unordered_map>
#include <string>

/* Brute Force Solution */
// int gridTraveler(int m, int n) {
//   if (m == 1 && n == 1) return 1;
//   if (m == 0 || n == 0) return 0;
//   return (gridTraveler(m-1, n) + gridTraveler(m, n-1));
// }

/* Memoize Solution */
/* now lets implement a memoized solution that stores the results of previous calculations:
*/
typedef unsigned long long ull;

ull gridTravelerHelper(int m, int n, std::unordered_map<std::string, ull>& memo) {
  //construct key as a string
  std::string key = std::to_string(m) + "," + std::to_string(n);

  //look for key in mao (previous solution)
  if (memo.contains(key)) return memo[key];

  //implement base cases
  if (m == 1 && n == 1) return 1;
  if (m == 0 || n == 0) return 0;

  //recursively call function
  memo[key] = gridTravelerHelper(m-1, n, memo) + gridTravelerHelper(m, n-1, memo);
  return memo[key];
}

ull gridTraveler(int m, int n) {
  std::unordered_map<std::string, ull> memo;
  return gridTravelerHelper(m, n, memo);
}

int main() {

  std::cout << "gridTraveler(1, 1) = " << gridTraveler(1, 1) << std::endl;
  std::cout << "gridTraveler(2, 3) = " << gridTraveler(2, 3) << std::endl;
  std::cout << "gridTraveler(3, 3) = " << gridTraveler(3, 3) << std::endl;
  std::cout << "gridTraveler(18, 18) = " << gridTraveler(18, 18) << std::endl;

  return 0;
}