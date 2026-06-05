/*Problem:
Write a function canConstruct(target, wordBank) that accepts a target string and an array of strings.

The function should return a boolean indicating whether or not the 'target' can be constructed by concatenating elements of the 'wordBank' array.

You may reuse elements of 'wordBank' as many times as needed.
*/

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using constStrVec = const std::vector<std::string>;
using mapStrBool = std::unordered_map<std::string, bool>;

/*Brute Force*/
// bool canConstruct(std::string target, constStrVec& wordBank) {
//   //add base case: built the word and there is nothing left to match
//   if (target.empty()) {
//     return true;
//   }

//   for (int i = 0; i < wordBank.size(); i++) {
//     const std::string& word = wordBank[i];

//     //1. check if the target starts with the current word in the bank: use .starts_with()
//     if (target.starts_with(word)) {
//       //2. slice off the matching prefix and get remainder: use .substr
//       std::string suffix = target.substr(word.length());

//       //3. recursive call with the new smaller target
//       if (canConstruct(suffix, wordBank) == true) {
//         return true;
//       }
//     }
//   }

//   return false;
// }

/*Memoized Solution*/
bool canConstructHelper(std::string target, constStrVec& wordBank, mapStrBool& memo) {
  //check the cache for a stored solution
  if (memo.contains(target)) return memo[target];

  //add base case: built the word and there is nothing left to match
  if (target.empty()) {
    return true;
  }

  for (int i = 0; i < wordBank.size(); i++) {
    const std::string& word = wordBank[i];

    //1. check if the target starts with the current word in the bank: use .starts_with()
    if (target.starts_with(word)) {
      //2. slice off the matching prefix and get remainder: use .substr
      std::string suffix = target.substr(word.length());

      //3. recursive call with the new smaller target
      if (canConstructHelper(suffix, wordBank, memo) == true) {
        memo[target] = true;
        return true;
      }
    }
  }
  memo[target] = false;
  return false;
}

bool canConstruct(std::string target, constStrVec& wordBank) {
  std::unordered_map<std::string, bool> memo;
  return canConstructHelper(target, wordBank, memo);
}

int main() {

  std::cout << std::boolalpha;

  std::cout << canConstruct("abcdef", {"ab", "abc", "cd", "def", "abcd"}) << std::endl;
  std::cout << canConstruct("skateboard", {"bo", "rd", "ate", "t", "ska", "sk", "boar"}) << std::endl;
  std::cout << canConstruct("enterapotentpot", {"a", "p", "ent", "enter", "ot", "o", "t"}) << std::endl;

  std::cout << "canConstruct(\"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeef\", ...) = " 
            << canConstruct("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeef", 
            {"e", "ee", "eee", "eeee", "eeeee", "eeeeee"}) << std::endl;

  return 0;
}