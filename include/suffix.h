#ifndef SUFFIX_H_
#define SUFFIX_H_

#include "../include/utility.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>  /* std::copy_if, std::max, std::lower_bound,
                         std::equal_range, std::make_pair, */
#include <numeric>    /* std::partial_sum */

#include <boost/algorithm/string/join.hpp>

#include <tbb/tbb.h>  /* tbb::parallel_for, tbb::parallel_sort */


namespace suffix {

typedef std::vector<std::pair<int, int>> Array;

Array buildArray(std::vector<std::string> const& names,
                 size_t const alphabet = 256) {
  
  std::string s = boost::algorithm::join(names, utility::getSeparator());
  size_t n = s.size();
  if (n == 0) return Array(1);
  int last = n - 1;
  size_t size = std::max(alphabet, n);
  size_t bucketSize = 1;

  std::vector<int> array(size), position(size), helper(size);

  tbb::parallel_for<size_t>(0, n, 1, [&](size_t i) noexcept {
      array[i] = i;
      position[i] = s[i];
    });

  auto cmp = [&](size_t i, size_t j) {
    if (position[i] != position[j])
      return position[i] < position[j];
    i += bucketSize;
    j += bucketSize;
    return (i < n && j < n) ? position[i] < position[j] : i > j;
  };

  for (;;bucketSize <<= 1) {
    tbb::parallel_sort(array.begin(), array.end(), cmp); 
    for (size_t i = 0; i != n - 1; ++i) {
      helper[i + 1] = helper[i] + cmp(array[i], array[i + 1]);
    }
    tbb::parallel_for<size_t>(0, n, 1, [&](size_t i) noexcept {
        position[array[i]] = helper[i];
      });
    if (helper[last] == last)
      break;
  }

  /*  adapt sa for string to sa for list of strings */
  std::vector<int> namePosition(names.size());
  tbb::parallel_for<size_t>(1, names.size(), 1, [&](size_t i) {
      namePosition[i] = names[i - 1].length();
    });
  std::partial_sum(namePosition.begin(),
                   namePosition.end(),
                   namePosition.begin(),
                   [](int i, int j) { return i + j + 1; });
  std::vector<std::pair<int, int> > result;

  for (int i : array) {
    auto it = std::lower_bound(namePosition.begin(), namePosition.end(), i);
    if (it == namePosition.end()) it = namePosition.end() - 1;
    size_t position = it - namePosition.begin();
    size_t offset   = i - namePosition[position];
    if (position < names.size() && offset < names[position].length()) {
      result.push_back(std::make_pair(position, offset));
    } 
  }
  
  return result;
}

namespace {
struct Comparator {
  explicit Comparator(std::vector<std::string> const& names) : m_names(names) {}
  bool operator() (std::string const& s, std::pair<int, int> const& p)
  { return s < m_names[p.first].substr(p.second, s.size()); }
  bool operator() (std::pair<int, int> const& p, std::string const& s)
  { return m_names[p.first].substr(p.second, s.size()) < s; }
 private:
  std::vector<std::string> const& m_names;
};
} // namespace anonymous


Array search(Array const& array,
             std::vector<std::string> const& names,
             std::string const& pattern) {
  auto range = std::equal_range(array.begin(), array.end(), pattern, Comparator(names));
  Array result;
  std::copy_if(range.first, range.second, std::back_inserter(result),
               [&](std::pair<int, int> const& p) noexcept {
                 return pattern == names[p.first].substr(p.second, pattern.size());
               });
  return result;
}

} // namespace suffix

#endif // SUFFIX_H_













