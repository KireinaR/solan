#include "box.h"
#include "format.h"
#include "colors.h"
#include <filesystem>
#include <iostream>
#include <algorithm>

void print_file_box(const std::vector<std::string> &filepaths)
{
    size_t name_width = std::string("Files to send").size();
    uint64_t total = 0;
    std::vector<std::pair<std::string, uint64_t>> rows;

    for (const auto &path : filepaths)
    {
        std::string name = std::filesystem::path(path).filename().string();
        uint64_t size = std::filesystem::exists(path) ? std::filesystem::file_size(path) : 0;
        total += size;
        name_width = std::max(name_width, name.size());
        rows.push_back({name, size});
    }

    size_t size_width = 10;
    size_t inner_width = name_width + size_width + 3;

    auto horizontal = [&](const char* left, const char* right)
    {
        std::cout << col::GRAY << left;
        for (size_t i = 0; i < inner_width + 2; ++i) std::cout << "─";
        std::cout << right << col::RESET << "\n";
    };

    horizontal("┌", "┐");

    std::string header = "Files to send";
    std::cout << col::GRAY << "│ " << col::RESET << col::BOLD << header << col::RESET;
    for (size_t i = header.size(); i < inner_width; ++i) std::cout << " ";
    std::cout << col::GRAY << " │" << col::RESET << "\n";

    horizontal("├", "┤");

    for (const auto &row : rows)
    {
        const std::string &name = row.first;
        std::string size_str = format_bytes(row.second);

        std::cout << col::GRAY << "│ " << col::RESET << name;
        for (size_t i = name.size(); i < name_width; ++i) std::cout << " ";
        std::cout << "   ";
        for (size_t i = size_str.size(); i < size_width; ++i) std::cout << " ";
        std::cout << size_str << col::GRAY << " │" << col::RESET << "\n";
    }

    horizontal("├", "┤");

    std::string total_str = format_bytes(total);
    std::cout << col::GRAY << "│ " << col::RESET << col::BOLD << "Total" << col::RESET;
    for (size_t i = std::string("Total").size(); i < name_width; ++i) std::cout << " ";
    std::cout << "   ";
    for (size_t i = total_str.size(); i < size_width; ++i) std::cout << " ";
    std::cout << col::BOLD << total_str << col::RESET << col::GRAY << " │" << col::RESET << "\n";

    horizontal("└", "┘");
}