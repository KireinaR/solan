#pragma once
#include <asio.hpp>

void run_server(asio::io_context &io, unsigned short port, bool interactive_exit = false);