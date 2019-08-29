#include "fpgrowth.cc"
#include <iomanip>
#include <cmath>

int main(int argc, char **argv) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);
  std::string st;
  FPgrowth fp(std::stod(argv[1]));
  std::ifstream fin;
  std::ofstream fout;
  fin.open(argv[2], std::ifstream::in);
  // first scan
  while (!getline(fin, st).eof()) {
    std::istringstream ss(st);
    std::string n;
    std::vector<int> data;
    while (getline(ss, n, ',')) {
      data.push_back(std::stoi(n));
    }
    fp.CountItemFrequency(data);
  }
  fp.CalculateMinSupportCount();
  fin.close();
  // second scan
  fin.open(argv[2], std::ifstream::in);
  FPTree fptree;
  while (!getline(fin, st).eof()) {
    std::istringstream ss(st);
    std::string n;
    std::vector<int> data;
    while (getline(ss, n, ',')) {
      data.push_back(std::stoi(n));
    }
    fp.SortAndTrimTransaction(data);
    fptree.InsertNodes(data);
  }
  auto result = fp.FindPattern(fptree);
  fout.open(argv[3], std::ofstream::out);
  for (const auto &tran : result) {
    std::string output;
    for (const auto &item : tran.first) {
      output += std::to_string(item) + ",";
    }
    output.pop_back();
    double sup = static_cast<double>(tran.second) / fp.total_transactions();
    fout << std::fixed << std::setprecision(4) << output << ":"
         << std::round(tran.second * 10000.f / fp.total_transactions()) / 10000
         << std::endl;
  }
}