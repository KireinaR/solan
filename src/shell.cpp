#include "shell.h"
#include "net/protocol.h"
#include "net/sender.h"
#include "net/receiver.h"
#include "net/discovery.h"
#include "net/archive.h"
#include "ui/colors.h"
#include "ui/box.h"
#include "ui/spinner.h"
#include <asio.hpp>
#include <atomic>
#include <thread>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

void print_banner()
{
    std::cout << "SOLAN - Send Over LAN - v0.1.0\n";
    std::cout << col::GRAY;
    for (int i = 0; i < 42; ++i)
        std::cout << "─";
    std::cout << col::RESET << "\n";
    std::cout << "Type \"help\" for commands.\n";
}

void print_section(const std::string &title)
{
    std::cout << col::BOLD << title << col::RESET << "\n";
}

void print_cmd(const std::string &cmd, const std::string &desc, size_t width)
{
    std::cout << "  " << col::CYAN << std::left << std::setw(static_cast<int>(width)) << cmd << col::RESET << desc << "\n";
}
void print_main_help()
{
    std::cout << "\n";
    print_section("General");
    print_cmd("help", "Show this help message", 14);
    print_cmd("exit", "Quit solan", 14);

    std::cout << "\n";
    print_section("Modes");
    print_cmd("mode", "Show the currently selected mode", 14);
    print_cmd("sm <mode>", "Switch to a mode (receive or send)", 14);

    std::cout << "\n";
    print_section("Examples");
    print_cmd("sm receive", "Start listening for incoming files", 14);
    print_cmd("sm send", "Discover peers and send files", 14);
}

void print_send_help()
{
    std::cout << "\n";
    print_section("Discovery");
    print_cmd("discover", "Scan the LAN for peers (5s)", 32);
    print_cmd("list", "Show the last discovery results", 32);

    std::cout << "\n";
    print_section("Transfer");
    print_cmd("send <n> <file1> [file2] ...", "Send file(s) to peer <n>, zipped automatically", 32);

    std::cout << "\n";
    print_section("Navigation");
    print_cmd("back / exmo", "Return to the main menu", 32);
    print_cmd("help", "Show this help message", 32);

    std::cout << "\n" << col::GRAY << "Typical flow: discover -> list -> send 1 file.txt" << col::RESET << "\n";
}

void receive_mode()
{
    asio::io_context io;
    std::atomic<bool> stop_flag{false};
    std::thread broadcaster(broadcast_presence, PORT, std::ref(stop_flag));

    std::cout << "  Ctrl+C   force-quit while idle\n";
    std::cout << "  exmo     exit receive mode (available after each transfer)\n";

    try
    {
        run_server(io, PORT, true);
    }
    catch (...)
    {
        stop_flag = true;
        broadcaster.join();
        throw;
    }

    stop_flag = true;
    broadcaster.join();
}

void send_mode()
{
    std::cout << "Send mode. Type \"help\" for commands, \"back\" to return.\n";

    std::vector<Peer> peers;
    std::string line;

    while (true)
    {
        std::cout << "\n"
                  << col::CYAN << "solan:send >> " << col::RESET;
        if (!std::getline(std::cin, line))
            return;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd.empty())
        {
            continue;
        }
        else if (cmd == "help")
        {
            print_send_help();
        }
        else if (cmd == "back" || cmd == "exmo")
        {
            return;
        }
        else if (cmd == "discover")
        {
            std::cout << "Scanning (5s)...\n";
            peers = discover_peers(5);
            if (peers.empty())
            {
                std::cout << "No peers found.\n";
            }
            else
            {
                std::cout << "Found " << peers.size() << " peer(s):\n";
                for (size_t i = 0; i < peers.size(); ++i)
                    std::cout << "  [" << (i + 1) << "] " << peers[i].ip
                              << "  (" << peers[i].hostname << ")\n";
            }
        }
        else if (cmd == "list")
        {
            if (peers.empty())
            {
                std::cout << "No peers discovered yet. Run \"discover\" first.\n";
            }
            else
            {
                std::cout << "Last discovery results:\n";
                for (size_t i = 0; i < peers.size(); ++i)
                    std::cout << "  [" << (i + 1) << "] " << peers[i].ip
                              << "  (" << peers[i].hostname << ")\n";
            }
        }
        else if (cmd == "send")
        {
            int index = 0;
            iss >> index;

            std::vector<std::string> files;
            std::string f;
            while (iss >> f)
            {
                files.push_back(f);
            }

            if (index < 1 || static_cast<size_t>(index) > peers.size() || files.empty())
            {
                std::cout << "Usage: send <n> <file1> [file2] [file3] ...   (run \"discover\" first to populate <n>)\n";
                continue;
            }

            print_file_box(files);

            std::string zip_path = "solan_transfer.zip";
            std::atomic<bool> spin_stop{false};
            std::thread spinner(run_spinner, "Packing files...", std::ref(spin_stop));

            try
            {
                create_zip(files, zip_path);
                spin_stop = true;
                spinner.join();

                asio::io_context io;
                run_client(io, peers[index - 1].ip, PORT, zip_path);

                std::filesystem::remove(zip_path);
            }
            catch (const std::exception &e)
            {
                spin_stop = true;
                if (spinner.joinable())
                    spinner.join();
                std::cerr << col::RED << "Send error: " << e.what() << col::RESET << std::endl;
                if (std::filesystem::exists(zip_path))
                {
                    std::filesystem::remove(zip_path);
                }
            }
        }
        else
        {
            std::cout << "Unknown command \"" << cmd << "\". Type \"help\" for a list of commands.\n";
        }
    }
}

void run_shell()
{
    std::string mode;
    std::string line;

    print_banner();

    while (true)
    {
        std::cout << "\n"
                  << col::CYAN << "solan >> " << col::RESET;
        if (!std::getline(std::cin, line))
            return;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd.empty())
        {
            continue;
        }
        else if (cmd == "help")
        {
            print_main_help();
        }
        else if (cmd == "mode")
        {
            std::cout << (mode.empty()
                              ? "No mode selected. Use \"sm <mode>\" to select."
                              : "Current mode: " + mode)
                      << std::endl;
        }
        else if (cmd == "sm")
        {
            std::string target;
            iss >> target;

            if (target == "receive")
            {
                mode = "receive";
                try
                {
                    receive_mode();
                }
                catch (const std::exception &e)
                {
                    std::cerr << col::RED << "Error: " << e.what() << col::RESET << std::endl;
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
                std::cout << "Usage: sm <mode>   (mode is one of: receive, send)\n";
            }
            else
            {
                std::cout << "Unknown mode \"" << target << "\". Valid modes: receive, send\n";
            }
        }
        else if (cmd == "exit")
        {
            std::cout << "bye.\n";
            return;
        }
        else
        {
            std::cout << "Unknown command \"" << cmd << "\". Type \"help\" for a list of commands.\n";
        }
    }
}