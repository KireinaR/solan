#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

// String formatting and measurement. Nothing here prints.

// SI byte formatting, matching portal's ByteCountSI (1000-based, "kB/MB/GB").
std::string format_bytes(uint64_t bytes);

// Go-style rounded duration: "9s", "1m09s", "1h02m09s".
// Empty when the duration is not yet meaningful.
std::string format_duration(double seconds);

// "1 object" / "3 objects".
std::string format_objects(size_t count);

namespace ui
{
    // Widths are counted in UTF-8 code points and ignore ANSI escape sequences,
    // so styled strings measure by what the reader actually sees.
    size_t visible_width(const std::string &text);

    // Right-truncates to max_cols, preserving escape sequences.
    std::string clip(const std::string &text, size_t max_cols);

    // Left-truncates plain text with a leading ellipsis, keeping the tail --
    // for file paths, where the end carries the meaning.
    std::string truncate_left(const std::string &plain, size_t max_cols);

    std::string pad_right(const std::string &plain, size_t cols);
}
