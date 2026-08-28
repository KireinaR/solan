#include "receiver.h"
#include "protocol.h"
#include <picosha2.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

void run_server(asio::io_context &io, unsigned short port)
{
    asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    std::cout << "Listening on port " << port << "..." << std::endl;

    asio::ip::tcp::socket socket(io);
    acceptor.accept(socket);
    std::cout << "Connected: " << socket.remote_endpoint().address().to_string() << std::endl;

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

    std::string temp_name = "received_" + filename + ".part";
    std::string final_name = "received_" + filename;

    picosha2::hash256_one_by_one hasher;
    hasher.init();

    {
        std::ofstream out("received_" + filename, std::ios::binary);
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
        throw std::runtime_error("Checksum mismatch — file corrupted in transit. Discarded.");
    }

    std::filesystem::rename(temp_name, final_name);
    std::cout << "Received " << file_size << " bytes. Verified and saved as '" << final_name << "'." << std::endl;
}