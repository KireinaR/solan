#include <asio.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include "net/protocol.h"
#include "net/sender.h"
#include "net/receiver.h"
#include "net/archive.h"
#include "ui/colors.h"
#include "shell.h"

void print_usage(const char *prog_name)
{
    std::cerr << "Usage:\n"
              << "  " << prog_name << "                              (launch interactive shell)\n"
              << "  " << prog_name << " server\n"
              << "  " << prog_name << " client <host> <file1> [file2] [file3] ...\n";
}

int main(int argc, char *argv[])
{
    enable_ansi_support();

    if (argc == 1)
    {
        run_shell();
        return 0;
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

            std::string host = argv[2];
            std::vector<std::string> files;
            for (int i = 3; i < argc; ++i)
            {
                files.push_back(argv[i]);
            }

            std::string zip_path = "solan_transfer.zip";
            create_zip(files, zip_path);

            run_client(io, host, PORT, zip_path);

            std::filesystem::remove(zip_path);
        }
        else
        {
            print_usage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << col::RED << "Error: " << e.what() << col::RESET << std::endl;
        return 1;
    }

    return 0;
}