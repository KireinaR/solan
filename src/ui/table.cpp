#include "ui/table.h"
#include "ui/theme.h"
#include "ui/text.h"
#include "ui/layout.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>

namespace
{
    // One column of gutter between fields, so the rows breathe the way the
    // site's hairline entries do.
    constexpr int GUTTER = 2;

    std::string cell(const std::string &text, const TableColumn &column)
    {
        const int inner = std::max(1, column.width);
        std::string body = ui::truncate_left(text, static_cast<size_t>(inner));
        const size_t width = ui::visible_width(body);

        if (column.right_align && width < static_cast<size_t>(inner))
        {
            return std::string(inner - width, ' ') + body;
        }
        return ui::pad_right(body, static_cast<size_t>(inner));
    }

    void render_row(const std::vector<TableColumn> &columns,
                    const std::vector<std::string> &values,
                    const char *color)
    {
        std::cout << ui::PAD;
        for (size_t c = 0; c < columns.size(); ++c)
        {
            if (c) std::cout << std::string(GUTTER, ' ');

            const std::string &value = c < values.size() ? values[c] : std::string();
            std::cout << color << cell(value, columns[c]) << col::RESET;
        }
        std::cout << "\n";
    }
}

void print_table(const std::vector<TableColumn> &columns,
                 const std::vector<std::vector<std::string>> &rows,
                 int footer_rows)
{
    if (columns.empty()) return;

    int total = static_cast<int>(columns.size() - 1) * GUTTER;
    for (const auto &c : columns) total += c.width;

    std::cout << ui::PAD;
    for (size_t c = 0; c < columns.size(); ++c)
    {
        if (c) std::cout << std::string(GUTTER, ' ');

        TableColumn header = columns[c];
        std::string title = header.title;
        for (auto &ch : title) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        std::cout << col::MUTED << cell(title, header) << col::RESET;
    }
    std::cout << "\n";

    ui::line(ui::rule(total));

    const int body_rows = static_cast<int>(rows.size()) - footer_rows;

    for (size_t r = 0; r < rows.size(); ++r)
    {
        if (footer_rows > 0 && static_cast<int>(r) == body_rows)
        {
            ui::line(ui::rule(total));
        }

        const bool is_footer = footer_rows > 0 && static_cast<int>(r) >= body_rows;
        render_row(columns, rows[r], is_footer ? col::BOLD : "");
    }
}

void print_file_table(const std::vector<std::string> &filepaths)
{
    const int total = ui::content_width();
    const int size_width = 12;
    const int name_width = std::max(8, total - size_width - GUTTER);

    std::vector<std::vector<std::string>> rows;
    uint64_t bytes = 0;

    for (const auto &path : filepaths)
    {
        const std::string name = std::filesystem::path(path).filename().string();
        const uint64_t size =
            std::filesystem::exists(path) ? std::filesystem::file_size(path) : 0;
        bytes += size;
        rows.push_back({name, format_bytes(size)});
    }

    int footer = 0;
    if (filepaths.size() > 1)
    {
        rows.push_back({std::to_string(filepaths.size()) + " objects", format_bytes(bytes)});
        footer = 1;
    }

    print_table({{"File", name_width}, {"Size", size_width, true}}, rows, footer);
}

void print_payload_table(const std::string &name, uint64_t size)
{
    const int total = ui::content_width();
    const int size_width = 12;
    const int name_width = std::max(8, total - size_width - GUTTER);

    print_table({{"Payload", name_width}, {"Size", size_width, true}},
                {{name, format_bytes(size)}});
}

void print_peer_table(const std::vector<std::pair<std::string, std::string>> &peers)
{
    const int total = ui::content_width();
    const int index_width = 3;
    const int addr_width = 18;
    const int host_width = std::max(8, total - index_width - addr_width - 2 * GUTTER);

    std::vector<std::vector<std::string>> rows;
    rows.reserve(peers.size());
    for (size_t i = 0; i < peers.size(); ++i)
    {
        rows.push_back({std::to_string(i + 1), peers[i].second, peers[i].first});
    }

    print_table({{"#", index_width}, {"Host", host_width}, {"Address", addr_width, true}}, rows);
}
