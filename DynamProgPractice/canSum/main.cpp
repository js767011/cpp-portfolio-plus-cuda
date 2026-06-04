/* Problem:
Write a function 'canSum(targetSum, numbers)' that takes in a targetSum
  and an array of numbers as arguments.

The function should return a boolean indicating whether or not it is
  possible to generate the targetSum using numbers from the array

You may use an element of the array as many times as needed.
You may assume that all input numbers are non-negative.
*/

#include <iostream>
#include <vector>
#include <unordered_map>

//Brute Force Solution
/*
m = target sum
n = array length
Time: O(n^m)
Space: O(m)
*/
// bool canSum(int targetSum, const std::vector<int>& numbers) {
//   if (targetSum == 0) return true;
//   if (targetSum < 0) return false;

//   for (int i = 0; i < numbers.size(); i++){
//     int num = numbers[i];
//     const int remainder = targetSum - num;
//     if (canSum(remainder, numbers) == true) {
//       return true;
//     }
//   }

//   return false;
// }

/* Memoize solution */
/* 
now lets improve the efficiency of the solution
can memoize the solution by tracking which numbers have known solutions
m = target sum
n = array length
Time: O(m*n)
Space: O(m)
*/
bool canSumHelper(int targetSum, const std::vector<int>& numbers, std::unordered_map<int, bool>& memo) {
  //check memo for the solution
  if (memo.contains(targetSum)) return memo[targetSum];

  //base cases
  if (targetSum == 0) return true;
  if (targetSum < 0) return false;

  for (int i = 0; i < numbers.size(); i++){
    int num = numbers[i];
    const int remainder = targetSum - num;
    if (canSumHelper(remainder, numbers, memo) == true) {
      memo[targetSum] = true;
      return true;
    }
  }
  memo[targetSum] = false;
  return false;
}

bool canSum(int targetSum, const std::vector<int>& numbers) {
  std::unordered_map<int, bool> memo;
  return canSumHelper(targetSum, numbers, memo);
}


int main() {

  std::cout << std::boolalpha;

  std::cout << "canSum(7, {2,3}) = " << canSum(7, {2,3}) << std::endl;
  std::cout << "canSum(7, {2,4}) = " << canSum(7, {2,4}) << std::endl;
  std::cout << "canSum(300, {7,14}) = " << canSum(300, {7,14}) << std::endl;
}