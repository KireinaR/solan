#include "sender.h"
#include "protocol.h"
#include <picosha2>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <stdexcept>

void run_client(asio::io_context &io, const std::string &host, unsigned short port, const std::string &filepath)
{
    std::ifstream in(filepath, std::ios::binary | std::ios::ate);
    if (!in)
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    asio::ip::tcp::socket socket(io);
    asio::ip::tcp::resolver resolver(io);
    asio::connect(socket, resolver.resolve(host, std::to_string(port)));
    std::cout << "Connected to " << host << ":" << port << std::endl;

    std::string filename = std::filesystem::path(filepath).filename().string();
    uint32_t name_len = static_cast<uint32_t>(filename.size());

    uint64_t file_size = static_cast<uint64_t>(in.tellg());
    in.seekg(0);

    asio::write(socket, asio::buffer(&name_len, sizeof(name_len)));
    asio::write(socket, asio::buffer(filename));
    asio::write(socket, asio::buffer(&file_size, sizeof(file_size)));

    picosha2::hash256_one_by_one hasher;
    hasher.init();

    std::vector<char> buffer(CHUNK_SIZE);
    uint64_t sent = 0;
    while (sent < file_size)
    {
        in.read(buffer.data(), std::min(CHUNK_SIZE, static_cast<size_t>(file_size - sent)));
        std::streamsize n = in.gcount();
        hasher.process(buffer.begin(), buffer.begin() + n);
        asio::write(socket, asio::buffer(buffer.data(), n));
        sent += n;
    }

    hasher.finish();
    std::vector<unsigned char> hash(HASH_SIZE);
    hasher.get_hash_bytes(hash.begin(), hash.end());
    asio::write(socket, asio::buffer(hash));

    std::cout << "Sent " << sent << " bytes ('" << filename << "'), hash appended." << std::endl;
}