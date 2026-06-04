/*Problem:
Write a funciton 'howSum(targetSum, numbers)' that takes in a
  targetSum and an array of numbers as arguments.

The function should return an array containing any combination of elements
  that add up to exactly the targetSum.
If there is no combination that adds up to the targetSum, then return null

If there are multiple combinations possible, you may return any single one.
*/

#include <iostream>
#include <vector>
#include <optional>
#include <unordered_map>

using OptionalIntVector = std::optional<std::vector<int>>;


/* Brute Force Solution
m = target sum
n = numbers.length

Time: O(n^m * m)
Size: O(m)
*/
// OptionalIntVector howSum (int targetSum, const std::vector<int>& numbers) {
//   if (targetSum == 0) return {std::vector<int>{}};
//   if (targetSum < 0) return std::nullopt;

//   for (int i = 0; i < numbers.size(); i++) {
//     int num = numbers[i];
//     const int remainder = targetSum - num;
//     OptionalIntVector remainderResult = howSum(remainder, numbers);
//     //check if the result is NOT null
//     if (remainderResult.has_value()){
//       //append the current number to the vector inside the optional
//       remainderResult->push_back(num);

//       //return the updated optional
//       return remainderResult;
//     }
//   }

//   return std::nullopt;
// }

/* Memoize solution */
/* 
now lets improve the efficiency of the solution
can memoize the solution by tracking which numbers have known solutions
m = target sum
n = array length
Time: O(n * m^2)
Space: O(m)
*/
OptionalIntVector canSumHelper(int targetSum, const std::vector<int>& numbers, std::unordered_map<int, OptionalIntVector>& memo) {
  //check memo for the solution
  if (memo.contains(targetSum)) return memo[targetSum];

  //base cases
  if (targetSum == 0) return {std::vector<int>{}};
  if (targetSum < 0) return std::nullopt;

  for (int i = 0; i < numbers.size(); i++) {
    int num = numbers[i];
    const int remainder = targetSum - num;
    OptionalIntVector remainderResult = canSumHelper(remainder, numbers, memo);
    //check if the result is NOT null
    if (remainderResult.has_value()){
      //append the current number to the vector inside the optional
      remainderResult->push_back(num);
      
      //update memo
      memo[targetSum] = remainderResult;
      //return the updated optional
      return remainderResult;
    }
  }
  //update memo
  memo[targetSum] = std::nullopt;
  return std::nullopt;
}

OptionalIntVector howSum(int targetSum, const std::vector<int>& numbers) {
  std::unordered_map<int, OptionalIntVector> memo;
  return canSumHelper(targetSum, numbers, memo);
}

int main() {

  //unpack and print result
  OptionalIntVector result = howSum(300, {1,2});

  std::cout << "howSum(300, {1,2}) = ";

  if (result.has_value()) {
    std::cout << "[ ";

    //loop through the vector inside the optional
    for (int num : result.value()) {
      std::cout << num << " ";
    }
    std::cout << "]" << std::endl;
  } else {
    std::cout << "null" << std::endl;
  }

  //std::cout << "howSum(7, {2,3}) = " << howSum(7, {2,3}) <<std::endl;
  return 0;
}