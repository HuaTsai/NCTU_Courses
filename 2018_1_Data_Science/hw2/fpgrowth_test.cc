#include "fpgrowth.cc"
#include <gtest/gtest.h>

std::vector<std::vector<int>> Testdata() {
  std::vector<std::vector<int>> data;
  data.push_back(std::vector<int>{0, 2, 3, 5, 6, 8, 12, 15});
  data.push_back(std::vector<int>{0, 1, 2, 5, 11, 12, 14});
  data.push_back(std::vector<int>{1, 5, 7, 9, 14});
  data.push_back(std::vector<int>{1, 2, 10, 15});
  data.push_back(std::vector<int>{0, 2, 4, 5, 11, 12, 15});
  return data;
}

int CountFrequency(int item, FPTree &fptree) {
  auto node = fptree.header.at(item);
  int result = node->frequency;
  while (node->next) {
    node = node->next;
    result += node->frequency;
  }
  return result;
}

void PrintResult(const std::map<std::set<int>, int, Comp> &result) {
  for (const auto &elem : result) {
    for (const auto &elem2 : elem.first) {
      std::cout << elem2 << " ";
    }
    std::cout << ": " << elem.second << std::endl;
  }
}

TEST(fpgrowth, SortAndTrimTransaction) {
  std::vector<std::vector<int>> data = Testdata();
  FPgrowth fp(0.6);
  for (const auto &elem : data) {
    fp.CountItemFrequency(elem);
  }
  fp.CalculateMinSupportCount();
  for (auto &elem : data) {
    fp.SortAndTrimTransaction(elem);
  }
  EXPECT_EQ(data[0], std::vector<int>({0, 12, 15, 2, 5}));
  EXPECT_EQ(data[1], std::vector<int>({0, 1 ,12, 2, 5}));
  EXPECT_EQ(data[2], std::vector<int>({1, 5}));
  EXPECT_EQ(data[3], std::vector<int>({1, 15, 2}));
  EXPECT_EQ(data[4], std::vector<int>({0, 12, 15, 2, 5}));
}

TEST(fpgrowth, TopDownLinkFPTree) {
  std::vector<std::vector<int>> data = Testdata();
  FPgrowth fp(0.6);
  for (const auto &elem : data) {
    fp.CountItemFrequency(elem);
  }
  fp.CalculateMinSupportCount();
  FPTree fptree;
  for (auto &elem : data) {
    fp.SortAndTrimTransaction(elem);
    fptree.InsertNodes(elem);
  }
  EXPECT_EQ(fptree.root->item, -1);
  EXPECT_EQ(fptree.root->parent, nullptr);

  EXPECT_EQ(fptree.root->children[0]->item, 5);
  EXPECT_EQ(fptree.root->children[0]->frequency, 4);

  EXPECT_EQ(fptree.root->children[0]->children[0]->item, 2);
  EXPECT_EQ(fptree.root->children[0]->children[0]->frequency, 3);

  EXPECT_EQ(fptree.root->children[0]->children[0]->children[1]->item, 12);
  EXPECT_EQ(fptree.root->children[0]->children[0]->children[1]->frequency, 1);

  EXPECT_EQ(fptree.root->children[0]->children[0]->children[1]->children[0]->item, 1);
  EXPECT_EQ(fptree.root->children[0]->children[0]->children[1]->children[0]->frequency, 1);

  EXPECT_EQ(fptree.root->children[0]->children[0]->children[1]->children[0]->children[0]->item, 0);
  EXPECT_EQ(fptree.root->children[0]->children[0]->children[1]->children[0]->children[0]->frequency, 1);

  EXPECT_EQ(fptree.root->children[0]->children[1]->item, 1);
  EXPECT_EQ(fptree.root->children[0]->children[1]->frequency, 1);

  EXPECT_EQ(fptree.root->children[1]->item, 2);
  EXPECT_EQ(fptree.root->children[1]->frequency, 1);

  EXPECT_EQ(fptree.root->children[1]->children[0]->item, 15);
  EXPECT_EQ(fptree.root->children[1]->children[0]->frequency, 1);

  EXPECT_EQ(fptree.root->children[1]->children[0]->children[0]->item, 1);
  EXPECT_EQ(fptree.root->children[1]->children[0]->children[0]->frequency, 1);
}

TEST(fpgrowth, LeftRightLinkFPTree) {
  std::vector<std::vector<int>> data = Testdata();
  FPgrowth fp(0.6);
  for (const auto &elem : data) {
    fp.CountItemFrequency(elem);
  }
  fp.CalculateMinSupportCount();
  FPTree fptree;
  for (auto &elem : data) {
    fp.SortAndTrimTransaction(elem);
    fptree.InsertNodes(elem);
  }
  EXPECT_EQ(CountFrequency(0, fptree), 3);
  EXPECT_EQ(CountFrequency(1, fptree), 3);
  EXPECT_EQ(CountFrequency(2, fptree), 4);
  EXPECT_EQ(CountFrequency(5, fptree), 4);
  EXPECT_EQ(CountFrequency(12, fptree), 3);
  EXPECT_EQ(CountFrequency(15, fptree), 3);
}

TEST(fpgrowth, IsSinglePath) {
  FPTree fptree;
  std::shared_ptr<FPNode> cur = fptree.root;
  FPgrowth fp(0.3);
  EXPECT_EQ(fp.IsSinglePath(fptree), true);
  cur->children.push_back(std::make_shared<FPNode>(1, cur));
  cur = cur->children[0];
  cur->children.push_back(std::make_shared<FPNode>(2, cur));
  EXPECT_EQ(fp.IsSinglePath(fptree), true);
  cur->children.push_back(std::make_shared<FPNode>(3, cur));
  EXPECT_EQ(fp.IsSinglePath(fptree), false);
}

TEST(fpgrowth, SinglePathPattern) {
  std::vector<int> data = {0, 1, 2};
  FPgrowth fp(1);
  for (int i = 0; i < 4; ++i) {
    fp.CountItemFrequency(data);
  }
  fp.CalculateMinSupportCount();
  FPTree fptree;
  fp.SortAndTrimTransaction(data);
  fptree.InsertNodes(data, 4);
  auto result = fp.FindPattern(fptree);
  EXPECT_EQ(result.size(), 7);
  EXPECT_EQ(result.at({0}), 4);
  EXPECT_EQ(result.at({1}), 4);
  EXPECT_EQ(result.at({2}), 4);
  EXPECT_EQ(result.at({0, 1}), 4);
  EXPECT_EQ(result.at({0, 2}), 4);
  EXPECT_EQ(result.at({1, 2}), 4);
  EXPECT_EQ(result.at({0, 1, 2}), 4);
}

TEST(fpgrowth, FindPattern) {
  std::vector<std::vector<int>> data = Testdata();
  FPgrowth fp(0.6);
  for (const auto &elem : data) {
    fp.CountItemFrequency(elem);
  }
  fp.CalculateMinSupportCount();
  FPTree fptree;
  for (auto &elem : data) {
    fp.SortAndTrimTransaction(elem);
    fptree.InsertNodes(elem);
  }
  auto result = fp.FindPattern(fptree);
  /*for (const auto &elem1 : result) {
    for (const auto &elem2 : elem1.first) {
      std::cout << static_cast<char>(elem2 + 'a') << " ";
    }
    std::cout << ": " << elem1.second << std::endl;
  }*/
  EXPECT_EQ(result.size(), 18);
  EXPECT_EQ(result.at({0}), 3);
  EXPECT_EQ(result.at({1}), 3);
  EXPECT_EQ(result.at({2}), 4);
  EXPECT_EQ(result.at({5}), 4);
  EXPECT_EQ(result.at({12}), 3);
  EXPECT_EQ(result.at({15}), 3);
  EXPECT_EQ(result.at({0, 2}), 3);
  EXPECT_EQ(result.at({0, 5}), 3);
  EXPECT_EQ(result.at({0, 12}), 3);
  EXPECT_EQ(result.at({2, 5}), 3);
  EXPECT_EQ(result.at({2, 12}), 3);
  EXPECT_EQ(result.at({2, 15}), 3);
  EXPECT_EQ(result.at({5, 12}), 3);
  EXPECT_EQ(result.at({0, 2, 5}), 3);
  EXPECT_EQ(result.at({0, 2, 12}), 3);
  EXPECT_EQ(result.at({0, 5, 12}), 3);
  EXPECT_EQ(result.at({2, 5, 12}), 3);
  EXPECT_EQ(result.at({0, 2, 5, 12}), 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
