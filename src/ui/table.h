#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct TableColumn
{
    std::string title;
    int width;
    bool right_align = false;
};

// Hairline table: uppercase meta labels, a single rule, flush rows. No box —
// the site sets its metadata the same way.
void print_table(const std::vector<TableColumn> &columns,
                 const std::vector<std::vector<std::string>> &rows,
                 int footer_rows = 0);

// File / Size table built from the paths on disk.
void print_file_table(const std::vector<std::string> &filepaths);

// Single-row payload summary, used on the receiving end.
void print_payload_table(const std::string &name, uint64_t size);

// # / Host / Address table for discovered peers.
void print_peer_table(const std::vector<std::pair<std::string, std::string>> &peers);
