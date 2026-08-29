#include "app/screens.h"
#include "ui/layout.h"
#include "ui/table.h"
#include "ui/text.h"
#include "ui/theme.h"
#include <iostream>
#include <utility>

namespace
{
    constexpr const char *VERSION = "v0.1.0";

    void section(const std::string &title)
    {
        ui::blank();
        ui::line(ui::label(title));
        ui::blank();
    }

    void command(const std::string &cmd, const std::string &desc, size_t width)
    {
        std::cout << ui::PAD << col::ACCENT << ui::pad_right(cmd, width) << col::RESET
                  << ui::help(desc) << "\n";
    }
}

namespace screens
{
    void banner()
    {
        ui::blank();
        ui::row(ui::bold(ui::spaced("solan")), ui::help(VERSION));
        ui::line(ui::help("Send Over LAN"));
        ui::blank();
        ui::separator();
        ui::blank();
        ui::line("Send a file to the laptop three feet away.");
        ui::line(ui::help("Two machines, one socket, no middleman."));
        ui::blank();
        ui::line(ui::element("help") + ui::help("  commands") + "     " +
                 ui::element("exit") + ui::help("  quit"));
    }

    void main_help()
    {
        section("General");
        command("help", "Show this help message", 16);
        command("exit", "Quit solan", 16);

        section("Modes");
        command("mode", "Show the currently selected mode", 16);
        command("sm <mode>", "Switch to a mode (receive or send)", 16);

        section("Examples");
        command("sm receive", "Listen for incoming files", 16);
        command("sm send", "Discover peers and send files", 16);
        ui::blank();
    }

    void send_help()
    {
        section("Discovery");
        command("discover", "Scan the LAN for peers (5s)", 32);
        command("list", "Show the last discovery results", 32);

        section("Transfer");
        command("send <n> <file> [file ...]", "Send to peer <n>, zipped automatically", 32);

        section("Navigation");
        command("back / exmo", "Return to the main menu", 32);
        command("help", "Show this help message", 32);

        ui::blank();
        ui::line(ui::help("Flow: ") + ui::element("discover") + ui::help(" → ") +
                 ui::element("list") + ui::help(" → ") + ui::element("send 1 file.txt"));
        ui::blank();
    }

    void prompt(const std::string &label)
    {
        ui::blank();
        std::cout << ui::PAD << ui::help(label) << col::ACCENT << " >> " << col::RESET << std::flush;
    }

    void peers(const std::vector<Peer> &found)
    {
        std::vector<std::pair<std::string, std::string>> rows;
        rows.reserve(found.size());
        for (const auto &p : found) rows.push_back({p.ip, p.hostname});

        ui::blank();
        print_peer_table(rows);
        ui::blank();
        ui::line(ui::help("Send with ") + ui::element("send <n> <file> ...") +
                 ui::help(", using the # column."));
    }

    void unknown_command(const std::string &cmd)
    {
        ui::blank();
        ui::line(ui::warning("Unknown command \"" + cmd + "\".") + ui::help(" Type ") +
                 ui::element("help") + ui::help(" for a list of commands."));
    }
}
