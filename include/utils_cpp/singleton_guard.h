/**
 * Created on: 1/11/17
 *     Author: xing
 */

#pragma once

#include "utils_cpp/common.h"

namespace utils
{
class SingletonGuard
{
 public:
  UTILS_DISALLOW_COPY_AND_ASSIGN(SingletonGuard)

  SingletonGuard() = delete;

  explicit SingletonGuard(const std::string &name);

  ~SingletonGuard();

 private:
  UTILS_READER(int, fd);
};

}  // namespace utils
