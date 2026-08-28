#include <asio.hpp>
#include <iostream>
#include "protocol.h"
#include "sender.h"
#include "receiver.h"

void print_usage(const char *prog_name)
{
    std::cerr << "Usage:\n"
              << "  " << prog_name << " server\n"
              << "  " << prog_name << " client <host> <filepath>\n";
}

int main(int argc, char *argv[])
{
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