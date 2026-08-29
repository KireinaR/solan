#include "net/archive.h"
#include <SimZip.h>
#include <filesystem>
#include <stdexcept>

void create_zip(const std::vector<std::string> &filepaths, const std::string &output_zip_path)
{
    SimZip zip(output_zip_path, SimZip::OpenMode::Create);

    for (const auto &path : filepaths)
    {
        if (!std::filesystem::exists(path))
        {
            throw std::runtime_error("File not found: " + path);
        }

        const std::string entry_name = std::filesystem::path(path).filename().string();
        zip.add(path, entry_name);
    }

    zip.save();
}

uint64_t total_size(const std::vector<std::string> &filepaths)
{
    uint64_t total = 0;
    for (const auto &path : filepaths)
    {
        if (std::filesystem::exists(path)) total += std::filesystem::file_size(path);
    }
    return total;
}
