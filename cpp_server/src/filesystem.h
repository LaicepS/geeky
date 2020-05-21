#pragma once

#include <experimental/filesystem>

namespace gky {

struct filesystem {
  using path = std::experimental::filesystem::path;

  virtual path root() = 0;

  virtual ~filesystem() {}
};
}
