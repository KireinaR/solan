#pragma once
#include "net/discovery.h"
#include <string>
#include <vector>

// The two transfer flows the shell drives. Each owns its spinners, its
// progress reporting, and the cleanup of the temporary archive.
namespace app
{
    // Packs the files, offers them to the peer, and sends on acceptance.
    // Reports failures on screen rather than throwing.
    void send_files(const Peer &peer, const std::vector<std::string> &files);

    // Listens for incoming transfers until the user leaves with "exmo",
    // broadcasting presence for discovery the whole time.
    void receive();
}
