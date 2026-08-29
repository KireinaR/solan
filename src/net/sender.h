#pragma once
#include <asio.hpp>
#include <cstddef>
#include <string>

// Describes what the payload on the wire really is, so the UI can report the
// original files rather than the archive standing in for them.
struct TransferInfo
{
    size_t object_count = 1;

    // True when filepath is an archive we built. A single file is sent as
    // itself, so this stays false and the file keeps its own name.
    bool compressed = false;
};

void run_client(asio::io_context &io, const std::string &host, unsigned short port,
                const std::string &filepath, const TransferInfo &info = {});
