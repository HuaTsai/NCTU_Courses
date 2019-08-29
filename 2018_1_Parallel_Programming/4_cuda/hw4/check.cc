#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

int main() {
  std::ifstream fin1, fin2;
  fin1.open("1.out", std::ios::in);
  fin2.open("2.out", std::ios::in);
  std::string str1, str2;
  std::vector<float> ans;
  for (int i = 0; i < 4; ++i) {
    getline(fin1, str1);
    getline(fin2, str2);
  }
  int i = 0;
  bool result = true;
  while (fin1 >> str1 && fin2 >> str2) {
    if (str1 == "Done.") {
      break;
    }
    ans.push_back(std::abs(std::stof(str1) - std::stof(str2)));
    if (std::abs(std::stof(str1) - std::stof(str2)) > 0.02) {
      result = false;
    }
  }
  std::cout << *std::max_element(ans.begin(), ans.end()) << std::endl;
  std::cout << std::boolalpha << result << std::endl;
}

