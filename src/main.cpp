#include <asio.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <filesystem>

constexpr size_t CHUNK_SIZE = 4096;

void run_server(asio::io_context &io, unsigned short port)
{
    asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)); // accepts connections
    std::cout << "Listening on port " << port << "." << std::endl;

    asio::ip::tcp::socket socket(io); // init io socket
    acceptor.accept(socket);          // accept io socket
    std::cout << "Connected: " << socket.remote_endpoint().address().to_string() << "\n";

    // read filename length, then filename
    uint32_t name_len = 0;
    asio::read(socket, asio::buffer(&name_len, sizeof(name_len)));
    std::string filename(name_len, '\0');
    asio::read(socket, asio::buffer(&filename[0], name_len));

    std::cout << filename << std::endl;

    // Read 8-byte file size header
    uint64_t file_size = 0;
    asio::read(socket, asio::buffer(&file_size, sizeof(file_size)));
    std::cout << "Incoming file size: " << file_size << " bytes" << std::endl;

    std::ofstream out("received_" + filename, std::ios::binary);
    std::vector<char> buffer(CHUNK_SIZE);
    uint64_t received = 0;

    while (received < file_size)
    {
        size_t to_read = std::min(CHUNK_SIZE, static_cast<size_t>(file_size - received));
        size_t n = asio::read(socket, asio::buffer(buffer.data(), to_read));
        out.write(buffer.data(), n);
        received += n;
    }

    std::cout << "Received " << received << " bytes. Saved as 'received_" << filename << "'" << std::endl;
}

void run_client(asio::io_context &io, const std::string &host, unsigned short port, const std::string &filepath)
{
    asio::ip::tcp::socket socket(io);
    asio::ip::tcp::resolver resolver(io);
    asio::connect(socket, resolver.resolve(host, std::to_string(port)));
    std::cout << "Connect to " << host << ":" << port << std::endl;

    std::ifstream in(filepath, std::ios::binary | std::ios::ate); // input file stream
    if (!in)
    {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return;
    }

    std::string filename = std::filesystem::path(filepath).filename().string(); // get the file name
    uint32_t name_len = static_cast<uint32_t>(filename.size());

    uint64_t file_size = static_cast<uint64_t>(in.tellg()); // tellg returns type streampos, so we cast it to uint64
    in.seekg(0);                                            // move pointer to start

    asio::write(socket, asio::buffer(&name_len, sizeof(name_len)));   // send file name length
    asio::write(socket, asio::buffer(filename));                      // send file name
    asio::write(socket, asio::buffer(&file_size, sizeof(file_size))); // send file size

    std::vector<char> buffer(CHUNK_SIZE);
    uint64_t sent = 0;
    while (sent < file_size)
    {
        in.read(buffer.data(), std::min(CHUNK_SIZE, static_cast<size_t>(file_size - sent)));
        std::streamsize n = in.gcount();
        asio::write(socket, asio::buffer(buffer.data(), n));
        sent += n;
    }

    std::cout << "Sent " << sent << " bytes." << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " server | client <host> <filepath>" << std::endl;
        return 1;
    }

    asio::io_context io;
    unsigned short port = 12345;

    std::string mode = argv[1];
    if (mode == "server")
    {
        run_server(io, port);
    }
    else if (mode == "client" && argc >= 4)
    {
        run_client(io, argv[2], port, argv[3]);
    }
    else
    {
        std::cerr << "Invalid args" << std::endl;
        return 1;
    }
    return 0;
}