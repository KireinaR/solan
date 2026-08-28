#include "shell.h"
#include "net/protocol.h"
#include "net/sender.h"
#include "net/receiver.h"
#include "net/discovery.h"
#include <asio.hpp>
#include <atomic>
#include <thread>
#include <iostream>
#include <iomanip>
#include <sstream>

void print_banner()
{
    std::cout << "SOLAN v0.1.0 - Send Over LAN\n";
    std::cout << "Type \"help\" for commands.\n\n";
}

void print_main_help()
{
    std::cout << "\nAvailable commands:\n\n";
    std::cout << "  " << std::left << std::setw(20) << "mode"
              << "Show the currently selected mode\n";
    std::cout << "  " << std::left << std::setw(20) << "sm <mode>"
              << "Switch to a mode. <mode> is one of: receive, send\n";
    std::cout << "  " << std::left << std::setw(20) << "help"
              << "Show this help message\n";
    std::cout << "  " << std::left << std::setw(20) << "exit"
              << "Quit solan\n";
    std::cout << "\nExamples:\n";
    std::cout << "  sm receive          Start listening for incoming files\n";
    std::cout << "  sm send             Enter send mode to discover peers and send files\n\n";
}

void print_send_help()
{
    std::cout << "\nSend mode commands:\n\n";
    std::cout << "  " << std::left << std::setw(24) << "discover"
              << "Scan the LAN for other SOLAN instances (5s scan)\n";
    std::cout << "  " << std::left << std::setw(24) << "list"
              << "Show the most recent discovery results\n";
    std::cout << "  " << std::left << std::setw(24) << "send <n> <filepath>"
              << "Send a file to peer number <n> from the list\n";
    std::cout << "  " << std::left << std::setw(24) << "back / exmo"
              << "Return to the main menu\n";
    std::cout << "  " << std::left << std::setw(24) << "help"
              << "Show this help message\n";
    std::cout << "\nTypical flow:\n";
    std::cout << "  1. discover\n";
    std::cout << "  2. list                       (optional, re-shows results)\n";
    std::cout << "  3. send 1 C:\\path\\to\\file.png\n\n";
}

void receive_mode()
{
    asio::io_context io;
    std::atomic<bool> stop_flag{false};
    std::thread broadcaster(broadcast_presence, PORT, std::ref(stop_flag));

    std::cout << "\nReceive mode - listening on port " << PORT << "\n";
    std::cout << "  Ctrl+C   force-quit while idle\n";
    std::cout << "  exmo     exit receive mode (available after each transfer)\n\n";

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
    std::cout << "\nSend mode. Type \"help\" for commands, \"back\" to return.\n";

    std::vector<Peer> peers;
    std::string line;

    while (true)
    {
        std::cout << "\nsolan:send >> ";
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
                std::cout << "\nFound " << peers.size() << " peer(s):\n";
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
                std::cout << "\nLast discovery results:\n";
                for (size_t i = 0; i < peers.size(); ++i)
                    std::cout << "  [" << (i + 1) << "] " << peers[i].ip
                              << "  (" << peers[i].hostname << ")\n";
            }
        }
        else if (cmd == "send")
        {
            int index;
            std::string filepath;
            iss >> index >> filepath;

            if (iss.fail() || index < 1 || static_cast<size_t>(index) > peers.size())
            {
                std::cout << "Usage: send <n> <filepath>   (run \"discover\" first to populate <n>)\n";
                continue;
            }

            asio::io_context io;
            try
            {
                run_client(io, peers[index - 1].ip, PORT, filepath);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Send error: " << e.what() << std::endl;
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
        std::cout << "solan >> ";
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
            print_main_help();
        }
        else if (cmd == "mode")
        {
            std::cout << (mode.empty()
                ? "No mode selected. Use \"sm <mode>\" to select."
                : "Current mode: " + mode) << std::endl;
        }
        else if (cmd == "sm")
        {
            std::string target;
            iss >> target;

            if (target == "receive")
            {
                mode = "receive";
                try { receive_mode(); }
                catch (const std::exception &e) { std::cerr << "Error: " << e.what() << std::endl; }
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