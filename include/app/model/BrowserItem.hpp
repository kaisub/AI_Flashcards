#pragma once

#include <filesystem>
#include <string>

namespace app::model {

struct BrowserItem {
    std::string displayName;
    std::filesystem::path fullPath;
    bool isDirectory = false;
};

} // namespace app::model