#include "app/transfer.h"
#include "net/archive.h"
#include "net/protocol.h"
#include "net/receiver.h"
#include "net/sender.h"
#include "ui/layout.h"
#include "ui/spinner.h"
#include "ui/status_view.h"
#include "ui/table.h"
#include "ui/text.h"
#include "ui/theme.h"
#include <asio.hpp>
#include <atomic>
#include <filesystem>
#include <thread>

namespace
{
    const char *ZIP_PATH = "solan_transfer.zip";
}

namespace app
{
    void send_files(const Peer &peer, const std::vector<std::string> &files)
    {
        ui::blank();
        ui::separator();
        ui::blank();
        print_file_table(files);
        ui::blank();

        // A single file is its own payload -- zipping one file only costs
        // time and buries its name inside an archive.
        const bool pack = files.size() > 1;
        std::string payload_path = files.front();

        try
        {
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

            asio::io_context io;
            run_client(io, peer.ip, PORT, payload_path, TransferInfo{files.size(), pack});

            if (pack) std::filesystem::remove(ZIP_PATH);
        }
        catch (const std::exception &e)
        {
            ui::task_fail(std::string("Send failed: ") + e.what());
            if (pack && std::filesystem::exists(ZIP_PATH))
            {
                std::filesystem::remove(ZIP_PATH);
            }
        }

        ui::blank();
        ui::separator();
    }

    void receive()
    {
        asio::io_context io;
        std::atomic<bool> stop_flag{false};
        std::thread broadcaster(broadcast_presence, PORT, std::ref(stop_flag));

        ui::blank();
        ui::separator();
        ui::blank();
        ui::line(ui::label("Receive mode"));
        ui::blank();
        ui::line(ui::help("Ctrl+C   force-quit while idle"));
        ui::line(ui::help("exmo     leave, offered after each transfer"));
        ui::blank();

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
}
