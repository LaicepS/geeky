#include <libgen.h>

#include <cstring>
#include <string>

#include "filesystem.h"

using namespace std;

namespace gky {

path dirname(path const& path) {
  char c_path[128];
  strcpy(c_path, path.c_str());
  ::dirname(c_path);
  return c_path;
}

}  // namespace gky
