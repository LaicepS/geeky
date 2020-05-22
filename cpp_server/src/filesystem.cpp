#include <libgen.h>

#include <cstring>
#include <fstream>
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

string load_file(string const& path) {
  ifstream f(path);
  string content;
  string curr_line;
  while (std::getline(f, curr_line))
    content += curr_line;

  return content;
}

}  // namespace gky
