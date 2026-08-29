#include "app/shell.h"
#include "app/screens.h"
#include "app/transfer.h"
#include "net/discovery.h"
#include "ui/layout.h"
#include "ui/spinner.h"
#include "ui/status_view.h"
#include "ui/theme.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    // Send mode: discover peers, then hand a selection to the transfer flow.
    void send_mode()
    {
        ui::blank();
        ui::separator();
        ui::blank();
        ui::line(ui::label("Send mode"));
        ui::blank();
        ui::line(ui::help("Type ") + ui::element("help") + ui::help(" for commands, ") +
                 ui::element("back") + ui::help(" to return."));

        std::vector<Peer> peers;
        std::string line;

        while (true)
        {
            screens::prompt("solan send");
            if (!std::getline(std::cin, line)) return;

            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd.empty())
            {
                continue;
            }
            else if (cmd == "help")
            {
                screens::send_help();
            }
            else if (cmd == "back" || cmd == "exmo")
            {
                return;
            }
            else if (cmd == "discover")
            {
                ui::blank();
                {
                    StatusView scanning(spin::WAITING, "Scanning the local network for peers");
                    peers = discover_peers(5);
                }

                if (peers.empty())
                {
                    ui::task_fail("No peers found. Is the other machine in receive mode?");
                }
                else
                {
                    ui::task_ok("Found " + std::to_string(peers.size()) +
                                (peers.size() == 1 ? " peer" : " peers"));
                    screens::peers(peers);
                }
            }
            else if (cmd == "list")
            {
                if (peers.empty())
                {
                    ui::blank();
                    ui::line(ui::warning("Nothing discovered yet.") + ui::help(" Run ") +
                             ui::element("discover") + ui::help(" first."));
                }
                else
                {
                    screens::peers(peers);
                }
            }
            else if (cmd == "send")
            {
                int index = 0;
                iss >> index;

                std::vector<std::string> files;
                std::string f;
                while (iss >> f) files.push_back(f);

                if (index < 1 || static_cast<size_t>(index) > peers.size() || files.empty())
                {
                    ui::blank();
                    ui::line(ui::warning("Usage: ") + ui::element("send <n> <file> [file ...]"));
                    ui::line(ui::help("Run ") + ui::element("discover") +
                             ui::help(" first to populate <n>."));
                    continue;
                }

                app::send_files(peers[index - 1], files);
            }
            else
            {
                screens::unknown_command(cmd);
            }
        }
    }

    // "sm <mode>" -- enters a mode and returns here when it exits.
    void switch_mode(const std::string &target, std::string &mode)
    {
        if (target == "receive")
        {
            mode = "receive";
            try
            {
                app::receive();
            }
            catch (const std::exception &e)
            {
                ui::task_fail(std::string("Error: ") + e.what());
            }
            mode.clear();
        }
        else if (target == "send")
        {
            mode = "send";
            send_mode();
            mode.clear();
        }
        else if (target.empty())
        {
            ui::blank();
            ui::line(ui::warning("Usage: ") + ui::element("sm <mode>") +
                     ui::help("   (receive or send)"));
        }
        else
        {
            ui::blank();
            ui::line(ui::warning("Unknown mode \"" + target + "\".") +
                     ui::help(" Valid modes: receive, send"));
        }
    }
}

void run_shell()
{
    std::string mode;
    std::string line;

    screens::banner();

    while (true)
    {
        screens::prompt("solan");
        if (!std::getline(std::cin, line)) return;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd.empty())
        {
            continue;
        }
        else if (cmd == "help")
        {
            screens::main_help();
        }
        else if (cmd == "mode")
        {
            ui::blank();
            if (mode.empty())
                ui::line(ui::help("No mode selected. Use ") + ui::element("sm <mode>") +
                         ui::help(" to pick one."));
            else
                ui::row(ui::label("Mode"), ui::element(mode));
        }
        else if (cmd == "sm")
        {
            std::string target;
            iss >> target;
            switch_mode(target, mode);
        }
        else if (cmd == "exit")
        {
            ui::blank();
            ui::line(ui::help("bye."));
            ui::blank();
            return;
        }
        else
        {
            screens::unknown_command(cmd);
        }
    }
}
