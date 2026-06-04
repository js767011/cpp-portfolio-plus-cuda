/* Problem:
Write a function 'bestSum(targetSum, numbers)' that takes in a targetSeum and an array of numbers as arguments

The function should return an array containing the shortest combination of numbers that add up to exactly the targetSum.

If there is a tie for shortest combination, you may return any one of the shortest.
*/

#include <iostream>
#include <vector>
#include <optional>
#include <unordered_map>

using OptionalIntVector = std::optional<std::vector<int>>;

/*Brute Force Solution*/
/*
m = target sum
n = numbers.length

Time: O(n^m * m)
Space O(m^2)
*/
// OptionalIntVector bestSum(int targetSum, const std::vector<int>& numbers) {
//   if (targetSum == 0) return std::vector<int> {};
//   if (targetSum < 0) return std::nullopt;

//   OptionalIntVector shortestCombination = std::nullopt;

//   for (int i = 0; i < numbers.size(); i++) {
//     int num = numbers[i];
//     const int remainder = targetSum - num;
//     OptionalIntVector remainderCombination = bestSum(remainder, numbers);

//     //check if the result is NOT null:
//     if (remainderCombination.has_value()) {
//       remainderCombination->push_back(num); 
//       OptionalIntVector combination = remainderCombination;
//       if (shortestCombination == std::nullopt || combination->size() < shortestCombination->size()) {
//         shortestCombination = combination;
//       }
//     }
//   }

//   return shortestCombination;
// }

/*Memoized Solution*/
/*
Time: O(m^2 * n)
Space: O(m^2)
*/
OptionalIntVector bestSumHelper(int targetSum, const std::vector<int>& numbers, std::unordered_map<int, OptionalIntVector>& memo) {
  //check memo for the solution
  if (memo.contains(targetSum)) return memo[targetSum];

  //base cases
  if (targetSum == 0) return {std::vector<int>{}};
  if (targetSum < 0) return std::nullopt;

  OptionalIntVector shortestCombination = std::nullopt;

  for (int i = 0; i < numbers.size(); i++) {
    int num = numbers[i];
    const int remainder = targetSum - num;
    OptionalIntVector remainderCombination = bestSumHelper(remainder, numbers, memo);
    //check if the result is NOT null
    if (remainderCombination.has_value()){
      //early branch pruning: if we already have a shorter path, ignore this one completely!
      if (shortestCombination.has_value() && remainderCombination->size() + 1 >= shortestCombination->size()) {
          continue; 
      }

      //append the current number to the vector inside the optional
      remainderCombination->push_back(num);
      shortestCombination = std::move(remainderCombination);

      // if (shortestCombination == std::nullopt || remainderCombination->size() < shortestCombination->size()) {
      //   shortestCombination = std::move(remainderCombination);
      // }
    }
  }
  //update memo
  memo[targetSum] = shortestCombination;
  return shortestCombination;
}

OptionalIntVector bestSum(int targetSum, const std::vector<int>& numbers) {
  std::unordered_map<int, OptionalIntVector> memo;
  return bestSumHelper(targetSum, numbers, memo);
}

int main() {

  //unpack and print result
  OptionalIntVector result = bestSum(100, {1,2,5,25});

  std::cout << "bestSum(100, {1,2,5,25}) = ";

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

  return 0;
}