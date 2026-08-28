#include <asio.hpp>
#include <iostream>
#include <atomic>

#include "net/protocol.h"
#include "net/sender.h"
#include "net/receiver.h"
#include "shell.h"
#include "net/discovery.h"

void print_usage(const char *prog_name)
{
    std::cerr << "Usage:\n"
              << "  " << prog_name << " server\n"
              << "  " << prog_name << " client <host> <filepath>\n";
}

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc == 1)
    {
        run_shell();
        return 0;
    }

    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];

    try
    {
        asio::io_context io;

        if (mode == "server")
        {
            run_server(io, PORT);
        }
        else if (mode == "client")
        {
            if (argc < 4)
            {
                print_usage(argv[0]);
                return 1;
            }
            run_client(io, argv[2], PORT, argv[3]);
        }
        else if (mode == "broadcast-test")
        {
            std::atomic<bool> stop_flag{false};
            std::cout << "Broadcasting as '" << asio::ip::host_name() << "'... Ctrl+C to stop." << std::endl;
            broadcast_presence(PORT, stop_flag);
        }
        else if (mode == "listen-test")
        {
            std::cout << "Listening for peers for 5 seconds..." << std::endl;
            auto peers = discover_peers(5);
            if (peers.empty())
            {
                std::cout << "No peers found." << std::endl;
            }
            else
            {
                for (const auto &p : peers)
                {
                    std::cout << "  " << p.ip << " (" << p.hostname << ")" << std::endl;
                }
            }
        }
        else
        {
            print_usage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}