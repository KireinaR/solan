#include <asio.hpp>
#include <iostream>

void run_server(asio::io_context& io, unsigned short port) {
    asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    std::cout << "Listening on port " << port << "." << std::endl;

    asio::ip::tcp::socket socket(io);
    acceptor.accept(socket);
    std::cout << "Connected: " << socket.remote_endpoint().address().to_string() << "\n";
}

void run_client(asio::io_context& io, const std::string& host, unsigned short port) {
    asio::ip::tcp::socket socket(io);
    asio::ip::tcp::resolver resolver(io);
    asio::connect(socket, resolver.resolve(host, std::to_string(port)));
    std::cout << "Connect to " << host << ":" << port << std::endl;
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " server | client <host>" << std::endl;
        return 1;
    }

    asio::io_context io;
    unsigned short port = 12345;

    std::string mode = argv[1];
    if(mode == "server") {
        run_server(io, port);
    }
    else if(mode == "client" && argc >= 3) {
        run_client(io, argv[2], port);
    }
    else {
        std::cerr << "Invalid args" << std::endl;
        return 1;
    }
    return 0;

}