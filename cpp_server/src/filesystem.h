#pragma once

#include <string>

namespace gky {

using path = std::string;

path dirname(path const& path);

std::string load_file(path const& path);
}
