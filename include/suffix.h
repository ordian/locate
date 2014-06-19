#ifndef SUFFIX_H_
#define SUFFIX_H_

#include "../include/utility.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>  /* std::for_each, std::fill, std::copy, std::max,
                         std::equal_range, std::back_inserter, std::copy */
#include <numeric>    /* std::partial_sum */

#include <boost/algorithm/string/join.hpp>

#include <tbb/tbb.h>  /* tbb::parallel_for */


namespace suffix {

typedef std::vector<std::pair<int, int>> Array;

Array buildArray(std::vector<std::string> const& names,
                 size_t const alphabet = 256) {
  
  std::string s = boost::algorithm::join(names, utility::getSeparator());
  size_t n = s.size();
  size_t size = std::max(alphabet, n);
  
  std::vector<int> permutation(size), bucket(size), count(size);
  for (unsigned char i : s) { ++count[i]; }
  std::partial_sum(count.begin(), count.begin() + alphabet, count.begin());
  for (size_t i = 0; i != n; ++i) { permutation[--count[static_cast<unsigned char>(s[i])]] = i; }
  bucket[permutation[0]] = 0;
  
  int buckets = 1;
  for (size_t i = 1; i != n; ++i) {
    if (s[permutation[i]] != s[permutation[i - 1]])
      ++buckets;
    bucket[permutation[i]] = buckets - 1;
  }

  std::vector<int> orderedBySecond(size), newNumber(size);

  for (size_t h = 0, k = 1 << h; k < n; ++h, k = 1 << h) {
    /* k = 2 ^ h */
    tbb::parallel_for<size_t>(0, n, 1, [&](size_t i) noexcept {
        orderedBySecond[i] = permutation[i] - k;
        if (orderedBySecond[i] < 0)
          orderedBySecond[i] += n;
      });
    std::fill(count.begin(), count.begin() + buckets, 0);
    for (size_t i = 0; i != n; ++i) { ++count[bucket[orderedBySecond[i]]]; } 
    std::partial_sum(count.begin(), count.begin() + buckets, count.begin());
    for (int i = n - 1; i >= 0; --i) {
      permutation[--count[bucket[orderedBySecond[i]]]] = orderedBySecond[i];
    }
    orderedBySecond[permutation[0]] = 0;
    buckets = 1;
    for (size_t i = 1; i != n; ++i) {
      int midCurrent  = (permutation[i]     + k) % n,
          midPrevious = (permutation[i - 1] + k) % n;
      if (bucket[permutation[i]] != bucket[permutation[i - 1]] ||
          bucket[midCurrent]     != bucket[midPrevious])
        ++buckets;
      orderedBySecond[permutation[i]] = buckets - 1;
    }
    std::copy(orderedBySecond.begin(), orderedBySecond.end(), bucket.begin());
  }
  while (size != n) { permutation.pop_back(); --size; }
  
  std::vector<int> namePosition(names.size());
  tbb::parallel_for<size_t>(1, names.size(), 1, [&](size_t i) {
      namePosition[i] = names[i - 1].length();
    });
  std::partial_sum(namePosition.begin(),
                   namePosition.end(),
                   namePosition.begin(),
                   [](int i, int j) { return i + j + 1; });
  std::vector<std::pair<int, int> > result;

  for (int i : permutation) {
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













