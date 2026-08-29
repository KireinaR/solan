#include "app/shell.h"
#include "net/archive.h"
#include "net/protocol.h"
#include "net/receiver.h"
#include "net/sender.h"
#include "ui/layout.h"
#include "ui/spinner.h"
#include "ui/status_view.h"
#include "ui/table.h"
#include "ui/terminal.h"
#include "ui/text.h"
#include "ui/theme.h"
#include <asio.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    const char *ZIP_PATH = "solan_transfer.zip";

    void print_usage(const char *prog_name)
    {
        const std::string prog = prog_name;

        ui::blank();
        ui::line(ui::label("Usage"));
        ui::blank();
        ui::line(ui::element(prog) +
                 ui::help("                                launch the interactive shell"));
        ui::line(ui::element(prog + " server") +
                 ui::help("                         listen for incoming files"));
        ui::line(ui::element(prog + " client <host> <file> [file ...]") +
                 ui::help("  send files to <host>"));
        ui::blank();
    }

    // Non-interactive send: same flow as the shell's, without the peer picker.
    void send_from_cli(asio::io_context &io, const std::string &host,
                       const std::vector<std::string> &files)
    {
        ui::blank();
        print_file_table(files);
        ui::blank();

        // A single file is its own payload; only a batch needs an archive.
        const bool pack = files.size() > 1;
        std::string payload_path = files.front();

        if (pack)
        {
            {
                StatusView packing(spin::COMPRESSING,
                                   "Compressing " + format_objects(files.size()) + " (" +
                                       format_bytes(total_size(files)) + "), preparing to send");
                create_zip(files, ZIP_PATH);
            }
            ui::task_ok("Compressed " + format_objects(files.size()) + " (" +
                        format_bytes(std::filesystem::file_size(ZIP_PATH)) + ")");
            payload_path = ZIP_PATH;
        }

        run_client(io, host, PORT, payload_path, TransferInfo{files.size(), pack});

        if (pack) std::filesystem::remove(ZIP_PATH);
        ui::blank();
    }
}

int main(int argc, char *argv[])
{
    enable_ansi_support();

    if (argc == 1)
    {
        run_shell();
        return 0;
    }

    const std::string mode = argv[1];

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

            const std::string host = argv[2];
            std::vector<std::string> files;
            for (int i = 3; i < argc; ++i) files.push_back(argv[i]);

            send_from_cli(io, host, files);
        }
        else
        {
            print_usage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        ui::task_fail(std::string("Error: ") + e.what());
        return 1;
    }

    return 0;
}
