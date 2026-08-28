#include "receiver.h"
#include "protocol.h"
#include "../ui/progress.h"

#include <picosha2.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <filesystem>

void run_server(asio::io_context &io, unsigned short port, bool interactive_exit)
{
    asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    std::cout << "Listening on port " << port << "..." << std::endl;

    while (true)
    {
        asio::ip::tcp::socket socket(io);
        acceptor.accept(socket);
        std::cout << "Connected: " << socket.remote_endpoint().address().to_string() << std::endl;

        try
        {
            uint32_t name_len = 0;
            asio::read(socket, asio::buffer(&name_len, sizeof(name_len)));

            if (name_len == 0 || name_len > MAX_FILENAME_LEN)
            {
                throw std::runtime_error("Received invalid filename length: " + std::to_string(name_len));
            }

            std::string filename(name_len, '\0');
            asio::read(socket, asio::buffer(&filename[0], name_len));

            uint64_t file_size = 0;
            asio::read(socket, asio::buffer(&file_size, sizeof(file_size)));
            std::cout << "Incoming: '" << filename << "' (" << file_size << " bytes)" << std::endl;

            std::cout << "Accept? [y/n]: ";
            char response;
            std::cin >> response;

            if (response != 'y' && response != 'Y')
            {
                unsigned char reject_byte = TRANSFER_REJECT;
                asio::write(socket, asio::buffer(&reject_byte, sizeof(reject_byte)));
                std::cout << "Rejected." << std::endl;
                // skip the rest of this iteration — fall through to the catch-free end of try, loop continues
            }
            else
            {
                unsigned char accept_byte = TRANSFER_ACCEPT;
                asio::write(socket, asio::buffer(&accept_byte, sizeof(accept_byte)));

                // everything from here down is your EXISTING logic, unchanged:
                // temp_name/final_name, hasher.init(), the ofstream block, hash comparison, rename

                std::string temp_name = "received_" + filename + ".part";
                std::string final_name = "received_" + filename;

                picosha2::hash256_one_by_one hasher;
                hasher.init();

                {
                    std::ofstream out(temp_name, std::ios::binary);
                    if (!out)
                    {
                        throw std::runtime_error("Cannot open output file: received_" + filename);
                    }

                    std::vector<char> buffer(CHUNK_SIZE);
                    uint64_t received = 0;
                    while (received < file_size)
                    {
                        size_t to_read = std::min(CHUNK_SIZE, static_cast<size_t>(file_size - received));
                        size_t n = asio::read(socket, asio::buffer(buffer.data(), to_read));
                        hasher.process(buffer.begin(), buffer.begin() + n);
                        out.write(buffer.data(), n);
                        received += n;
                        print_progress(received, file_size, "Receiving");
                    }
                }

                finish_progress();
                hasher.finish();
                std::vector<unsigned char> computed_hash(HASH_SIZE);
                hasher.get_hash_bytes(computed_hash.begin(), computed_hash.end());

                std::vector<unsigned char> received_hash(HASH_SIZE);
                asio::read(socket, asio::buffer(received_hash));

                if (computed_hash != received_hash)
                {
                    std::filesystem::remove(temp_name);
                    throw std::runtime_error("CHECKSUM_MISMATCH: File corrupted in transit. Discarded.");
                }

                if (std::filesystem::exists(final_name))
                {
                    std::filesystem::remove(final_name);
                }
                std::filesystem::rename(temp_name, final_name);
                std::cout << "Received " << file_size << " bytes. Verified and saved as '" << final_name << "'." << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Transfer error: " << e.what() << std::endl;
        }

        std::cout << "Waiting for next connection..." << std::endl;

        if (interactive_exit)
        {
            std::cout << "Press Enter to keep receiving, or type 'exmo' to exit: ";
            std::string line;
            std::getline(std::cin, line);
            if (line == "exmo")
            {
                std::cout << "Exiting receive mode." << std::endl;
                return;
            }
        }
        else
        {
            std::cout << "Waiting for next connection..." << std::endl;
        }
    }
}