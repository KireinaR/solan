#pragma once
#include "net/discovery.h"
#include <string>
#include <vector>

// Everything the shell puts on screen that isn't a transfer: the banner, the
// help screens, the prompts, and the peer list. Presentation only -- nothing
// here touches the network or the filesystem.
namespace screens
{
    void banner();

    void main_help();
    void send_help();

    // Writes the prompt without a trailing newline, ready for getline.
    void prompt(const std::string &label);

    // Peer table plus the hint on how to address a row.
    void peers(const std::vector<Peer> &found);

    void unknown_command(const std::string &cmd);
}
