#include "net/receiver.h"
#include "net/protocol.h"
#include "net/discovery.h"
#include "ui/table.h"
#include "ui/theme.h"
#include "ui/text.h"
#include "ui/spinner.h"
#include "ui/layout.h"
#include "ui/status_view.h"
#include <picosha2.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
    std::string listen_address(unsigned short port)
    {
        try
        {
            return get_local_ip() + ":" + std::to_string(port);
        }
        catch (const std::exception &)
        {
            return "port " + std::to_string(port);
        }
    }
}

void run_server(asio::io_context &io, unsigned short port, bool interactive_exit)
{
    asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    const std::string address = listen_address(port);

    while (true)
    {
        asio::ip::tcp::socket socket(io);
        {
            StatusView waiting(spin::WAITING, "Listening on " + ui::element(address));
            acceptor.accept(socket);
        }

        const std::string peer = socket.remote_endpoint().address().to_string();
        ui::task("Incoming from " + ui::element(peer));

        try
        {
            uint32_t name_len = 0;
            asio::read(socket, asio::buffer(&name_len, sizeof(name_len)));

            if (name_len == 0 || name_len > MAX_FILENAME_LEN)
            {
                throw std::runtime_error("Received invalid filename length: " +
                                         std::to_string(name_len));
            }

            std::string filename(name_len, '\0');
            asio::read(socket, asio::buffer(&filename[0], name_len));

            uint64_t file_size = 0;
            asio::read(socket, asio::buffer(&file_size, sizeof(file_size)));

            ui::blank();
            print_payload_table(filename, file_size);
            ui::blank();

            ui::line("Accept this transfer?  " + ui::help("(y) accept   (n) reject"));
            std::cout << ui::PAD << col::ACCENT << ">> " << col::RESET << std::flush;

            char response = 'n';
            std::cin >> response;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ui::blank();

            if (response != 'y' && response != 'Y')
            {
                unsigned char reject_byte = TRANSFER_REJECT;
                asio::write(socket, asio::buffer(&reject_byte, sizeof(reject_byte)));
                ui::task_fail("Rejected. Nothing was written to disk.");
            }
            else
            {
                unsigned char accept_byte = TRANSFER_ACCEPT;
                asio::write(socket, asio::buffer(&accept_byte, sizeof(accept_byte)));

                const std::string temp_name = "received_" + filename + ".part";
                const std::string final_name = "received_" + filename;

                picosha2::hash256_one_by_one hasher;
                hasher.init();

                // The status block stays alive through verification, so a
                // failed checksum tears it down instead of leaving a record
                // of a transfer that did not survive.
                StatusView receiving(spin::RECEIVING,
                                     "Receiving " + filename + " " +
                                         ui::help("(" + format_bytes(file_size) + ")"));
                receiving.track_bytes(file_size);

                {
                    std::ofstream out(temp_name, std::ios::binary);
                    if (!out)
                    {
                        throw std::runtime_error("Cannot open output file: " + temp_name);
                    }

                    std::vector<char> buffer(CHUNK_SIZE);
                    uint64_t received = 0;
                    while (received < file_size)
                    {
                        const size_t to_read =
                            std::min(CHUNK_SIZE, static_cast<size_t>(file_size - received));
                        const size_t n = asio::read(socket, asio::buffer(buffer.data(), to_read));
                        hasher.process(buffer.begin(), buffer.begin() + n);
                        out.write(buffer.data(), n);
                        received += n;
                        receiving.set_bytes(received);
                    }
                }

                hasher.finish();
                std::vector<unsigned char> computed_hash(HASH_SIZE);
                hasher.get_hash_bytes(computed_hash.begin(), computed_hash.end());

                std::vector<unsigned char> received_hash(HASH_SIZE);
                asio::read(socket, asio::buffer(received_hash));

                if (computed_hash != received_hash)
                {
                    std::filesystem::remove(temp_name);
                    throw std::runtime_error(
                        "Checksum mismatch. The file was corrupted in transit and discarded.");
                }

                if (std::filesystem::exists(final_name))
                {
                    std::filesystem::remove(final_name);
                }
                std::filesystem::rename(temp_name, final_name);

                receiving.finish("Received " + filename + ", SHA-256 verified");
                ui::blank();
                ui::task_ok("Saved as " + final_name);
            }
        }
        catch (const std::exception &e)
        {
            ui::task_fail(std::string("Transfer failed: ") + e.what());
        }

        ui::blank();
        ui::separator();

        if (interactive_exit)
        {
            ui::blank();
            ui::line(ui::help("Enter    keep receiving"));
            ui::line(ui::help("exmo     leave receive mode"));
            std::cout << ui::PAD << col::ACCENT << ">> " << col::RESET << std::flush;

            std::string line;
            std::getline(std::cin, line);
            if (line == "exmo")
            {
                ui::blank();
                ui::task("Leaving receive mode.");
                return;
            }
            ui::blank();
        }
    }
}
