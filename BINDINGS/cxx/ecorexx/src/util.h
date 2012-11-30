#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

/// create std::string from any number
template <typename T>
std::string toString(const T &thing, int w = 0, int p = 0)
{
  std::ostringstream os;
  os << std::setw(w) << std::setprecision(p) << thing;
  return os.str();
}

template <typename T>
std::string toStringWide(const T &thing, int w = 0)
{
  std::ostringstream os;
  os << std::setw(w) << thing;
  return os.str();
}

#endif // UTIL_H
