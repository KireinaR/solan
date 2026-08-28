#include "discovery.h"
#include "protocol.h"
#include <asio.hpp>
#include <chrono>
#include <thread>
#include <set>
#include <iostream>

std::string get_local_ip()
{
    asio::io_context io;
    asio::ip::udp::socket socket(io, asio::ip::udp::v4());
    socket.connect(asio::ip::udp::endpoint(asio::ip::make_address("8.8.8.8"), 53));
    return socket.local_endpoint().address().to_string();
}

void broadcast_presence(unsigned short tcp_port, std::atomic<bool> &stop_flag)
{
    try
    {
        asio::io_context io;
        std::string local_ip = get_local_ip();

        asio::ip::udp::socket socket(io, asio::ip::udp::endpoint(asio::ip::make_address(local_ip), 0));
        socket.set_option(asio::socket_base::broadcast(true));

        asio::ip::udp::endpoint broadcast_endpoint(asio::ip::address_v4::broadcast(), DISCOVERY_PORT);
        std::string message = std::string(DISCOVERY_MAGIC) + "|" + asio::ip::host_name() + "|" + std::to_string(tcp_port);

        while (!stop_flag.load())
        {
            socket.send_to(asio::buffer(message), broadcast_endpoint);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "BROADCAST_ERROR: " << e.what() << std::endl;
    }
}

std::vector<Peer> discover_peers(int timeout_seconds)
{
    std::vector<Peer> peers;
    std::set<std::string> seen_ips;

    asio::io_context io;
    asio::ip::udp::socket socket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), DISCOVERY_PORT));
    socket.set_option(asio::socket_base::reuse_address(true));
    socket.non_blocking(true);

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds);
    std::vector<char> buf(256);

    while (std::chrono::steady_clock::now() < deadline)
    {
        asio::ip::udp::endpoint sender_endpoint;
        asio::error_code ec;
        size_t len = socket.receive_from(asio::buffer(buf), sender_endpoint, 0, ec);

        if (!ec && len > 0)
        {
            std::string msg(buf.data(), len);
            size_t p1 = msg.find('|');
            size_t p2 = msg.rfind('|');
            if (p1 != std::string::npos && p2 != std::string::npos && p1 != p2 &&
                msg.substr(0, p1) == DISCOVERY_MAGIC)
            {
                std::string ip = sender_endpoint.address().to_string();
                std::string hostname = msg.substr(p1 + 1, p2 - p1 - 1);
                if (seen_ips.insert(ip).second)
                {
                    peers.push_back({ip, hostname});
                }
            }
        }
        else if (ec == asio::error::would_block)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    return peers;
}