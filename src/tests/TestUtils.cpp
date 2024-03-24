
#include "TestUtils.h"
#include <fstream>


namespace fs = std::filesystem;

fs::path getTestRoot()
{
    auto curr = fs::current_path();
    fs::path ret;

    for (const auto& sub : curr) {
        ret /= sub;

        if (sub == "gbemu")
            break;
    }

    ret /= "test-files";

    return ret;
}


std::vector<uint8_t> readFile(const fs::path& path)
{
    if (fs::exists(path) && fs::is_regular_file(path)) {
        auto size = fs::file_size(path);

        std::vector<uint8_t> data;
        data.resize(size);

        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        ifs.read((char*)data.data(), size);

        return data;
    }
    else {
        return {};
    }
}

