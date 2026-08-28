#pragma once
#include <asio.hpp>
#include <string>

void run_client(asio::io_context &io, const std::string &host, unsigned short port, const std::string &filepath);