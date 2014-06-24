#ifndef UTILITY_H_
#define UTILITY_H_

#include <fstream>
#include <cassert>

#include <boost/filesystem.hpp>


namespace utility {

std::string getSeparator() {
  static boost::filesystem::path slash("/");
  static std::string preferredSlash = slash.make_preferred().native();
  return preferredSlash;
}


void read(std::ifstream &in, size_t & num) {
  num = 0;
  for (int k = 3; k != -1; --k) {
    unsigned char c = in.get();
    num += c * (1 << (8 * k));
  }
}

void read(std::ifstream &in, int & num) {
  num = 0;
  for (int k = 3; k != -1; --k) {
    unsigned char c = in.get();
    num += c * (1 << (8 * k));
  }
}


void read(std::ifstream &in, std::string & s) {
  size_t size = 0;
  read(in, size);
  std::vector<char> buffer(size + 1, '\0');
  in.read(&buffer[0], size);
  s = std::string(&buffer[0]);
}

void read(std::ifstream &in, std::pair<int, int> & p) {
  p.first = 0, p.second = 0;
  read(in, p.first);
  read(in, p.second);
}

template<typename T>
void read(std::ifstream &in, std::vector<T> & v) {
  size_t size = 0;
  read(in, size);
  v.resize(size);
  for (auto &a : v)
    read(in, a);
}



void write(std::ofstream &out, size_t const& num) {
  assert(num <= 4294967295); /* 2^32 - 1 */
  for (int k = 3; k != -1; --k) {
    out.put(static_cast<unsigned char>((num >> (8 * k))  % 256));
  }
}


void write(std::ofstream &out, int const& num) {
  for (int k = 3; k != -1; --k) {
    out.put(static_cast<unsigned char>((num >> (8 * k))  % 256));
  }
}

void write(std::ofstream &out, std::string const& s) {
  write(out, s.size());
  out.write(s.c_str(), s.size());
}


void write(std::ofstream &out, std::pair<int, int> const& p) {
  write(out, p.first);
  write(out, p.second);
}

template<typename T>
void write(std::ofstream &out, std::vector<T> const& v) {
  size_t size = v.size();
  write(out, size);
  for (auto const& a : v)
    write(out, a);
}



} // namespace utility

#endif // UTILITY_H_
