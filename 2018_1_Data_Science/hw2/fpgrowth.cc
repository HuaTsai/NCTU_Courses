#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

auto comp = [](const std::set<int> &s1, const std::set<int> &s2) {
  if (s1.size() != s2.size()) {
    return s1.size() < s2.size();
  } else {
    return s1 < s2;
  }
};
typedef decltype(comp) Comp;

struct FPNode {
  FPNode(const int &item, const std::shared_ptr<FPNode> &parent,
         const int &frequency);
  int item;
  int frequency;
  std::shared_ptr<FPNode> parent;
  std::vector<std::shared_ptr<FPNode>> children;
  std::shared_ptr<FPNode> next;
};

FPNode::FPNode(const int &item, const std::shared_ptr<FPNode> &parent,
               const int &frequency = 1)
    : item(item), parent(parent), frequency(frequency), next(nullptr) {}

struct FPTree {
  FPTree();
  void InsertNodes(std::vector<int> &trans, int repeat);
  std::shared_ptr<FPNode> root;
  std::map<int, std::shared_ptr<FPNode>> header;
};

FPTree::FPTree() : root(std::make_shared<FPNode>(-1, nullptr)) {}

void FPTree::InsertNodes(std::vector<int> &trans, int repeat = 1) {
  std::shared_ptr<FPNode> current_node = root;
  for (auto item = trans.crbegin(); item != trans.crend(); ++item) {
    // TODO(HuaTsai): array accelerate
    auto pos = std::find_if(
        current_node->children.cbegin(), current_node->children.cend(),
        [&](const std::shared_ptr<FPNode> &x) { return x->item == *item; });
    if (pos == current_node->children.cend()) {
      auto node = std::make_shared<FPNode>(*item, current_node, repeat);
      if (header.count(*item)) {
        // TODO(HuaTsai): accelerate by tmp variable
        auto tmp_node = header.at(*item);
        while (tmp_node->next) {
          tmp_node = tmp_node->next;
        }
        tmp_node->next = node;
      } else {
        header.insert(std::make_pair(*item, node));
      }
      current_node->children.push_back(node);
      current_node = current_node->children.back();
    } else {
      (*pos)->frequency += repeat;
      current_node = *pos;
    }
  }
}

class FPgrowth {
 public:
  FPgrowth(double min_support);
  int total_transactions();
  void CountItemFrequency(const std::vector<int> &trans);
  void SortAndTrimTransaction(std::vector<int> &trans);
  void CalculateMinSupportCount();
  std::map<std::set<int>, int, Comp> FindPattern(FPTree fptree);
  bool IsSinglePath(FPTree fptree);

 private:
  double min_support_;
  int min_support_count_;
  int total_transactions_;
  std::map<int, int> item_frequency_;
};

FPgrowth::FPgrowth(double min_support)
    : min_support_(min_support), total_transactions_(0) {}

int FPgrowth::total_transactions() { return total_transactions_; }

void FPgrowth::CountItemFrequency(const std::vector<int> &trans) {
  for (const auto &elem : trans) {
    if (item_frequency_.count(elem)) {
      ++item_frequency_.at(elem);
    } else {
      item_frequency_.insert(std::make_pair(elem, 1));
    }
  }
  ++total_transactions_;
}

void FPgrowth::CalculateMinSupportCount() {
  min_support_count_ = static_cast<int>(min_support_ * total_transactions_);
}

void FPgrowth::SortAndTrimTransaction(std::vector<int> &trans) {
  std::sort(trans.begin(), trans.end(), [&](int x, int y) {
    if (item_frequency_.at(x) == item_frequency_.at(y)) {
      return x < y;
    } else {
      return item_frequency_.at(x) < item_frequency_.at(y);
    }
  });
  auto pos = std::find_if(trans.begin(), trans.end(), [&](int x) {
    return item_frequency_.at(x) >= min_support_count_;
  });
  trans.erase(trans.begin(), pos);
}

std::map<std::set<int>, int, Comp> FPgrowth::FindPattern(FPTree fptree) {
  std::map<std::set<int>, int, Comp> conditional_pattern(comp);
  if (IsSinglePath(fptree)) {
    std::shared_ptr<FPNode> cur = fptree.root;
    while (!cur->children.empty()) {
      cur = cur->children.front();
      const int &cur_item = cur->item;
      const int &cur_frequecy = cur->frequency;
      conditional_pattern.insert(
          std::make_pair(std::set<int>{cur_item}, cur_frequecy));
      for (const auto &elem : conditional_pattern) {
        if (elem.first != std::set<int>{cur_item}) {
          auto tmp_set = elem.first;
          tmp_set.insert(cur_item);
          conditional_pattern.insert(std::make_pair(tmp_set, cur_frequecy));
        }
      }
    }
    return conditional_pattern;
  } else {
    // TODO(HuaTsai): array accelerate
    for (const auto &elem : fptree.header) {
      auto item_cur = elem.second;
      std::map<int, int> conditional_frequency;
      std::vector<std::pair<std::vector<int>, int>> conditional_transactions;
      int one_item_frequency = 0;
      while (item_cur) {
        const int &item_frequency = item_cur->frequency;
        one_item_frequency += item_frequency;
        std::pair<std::vector<int>, int> conditional_transaction_frequency;
        conditional_transaction_frequency.second = item_frequency;
        auto parent_cur = item_cur->parent;
        while (parent_cur->item != -1) {
          const int &item = parent_cur->item;
          conditional_transaction_frequency.first.push_back(item);
          if (conditional_frequency.count(item)) {
            conditional_frequency.at(item) += item_frequency;
          } else {
            conditional_frequency.insert(std::make_pair(item, item_frequency));
          }
          parent_cur = parent_cur->parent;
        }
        if (!conditional_transaction_frequency.first.empty()) {
          conditional_transactions.push_back(conditional_transaction_frequency);
        }
        item_cur = item_cur->next;
      }
      FPTree conditional_fptree;
      for (auto &tran : conditional_transactions) {
        std::vector<int> tmp;
        std::copy_if(tran.first.begin(), tran.first.end(),
                     std::back_inserter(tmp), [&](const int &x) {
                       return conditional_frequency.at(x) >= min_support_count_;
                     });
        conditional_fptree.InsertNodes(tmp, tran.second);
      }
      auto pattern = FindPattern(conditional_fptree);
      for (const auto &pattern_set : pattern) {
        auto tmp_set = pattern_set.first;
        tmp_set.insert(elem.first);
        conditional_pattern.insert(std::make_pair(tmp_set, pattern_set.second));
      }
      conditional_pattern.insert(
          std::make_pair(std::set<int>{elem.first}, one_item_frequency));
    }
    return conditional_pattern;
  }
}

bool FPgrowth::IsSinglePath(FPTree fptree) {
  std::shared_ptr<FPNode> cur = fptree.root;
  while (cur->children.size()) {
    if (cur->children.size() > 1) {
      return false;
    }
    cur = cur->children.front();
  }
  return true;
}
