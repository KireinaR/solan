#pragma once
#include <atomic>
#include <string>
#include <vector>

struct Peer
{
    std::string ip;
    std::string hostname;
};

void broadcast_presence(unsigned short tcp_port, std::atomic<bool> &stop_flag);
std::vector<Peer> discover_peers(int timeout_seconds);