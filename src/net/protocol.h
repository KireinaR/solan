#pragma once
#include <cstdint>
#include <cstddef>

constexpr unsigned short PORT = 48562;
constexpr size_t CHUNK_SIZE = 4096;
constexpr uint32_t MAX_FILENAME_LEN = 4096;
constexpr size_t HASH_SIZE = 32; // SHA-256 digest size

constexpr unsigned short DISCOVERY_PORT = 48563;
constexpr const char* DISCOVERY_MAGIC = "SOLAN";

constexpr unsigned char TRANSFER_ACCEPT = 1;
constexpr unsigned char TRANSFER_REJECT = 0;

constexpr int PROGRESS_BAR_WIDTH = 40;