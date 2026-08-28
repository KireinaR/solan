# SOLAN: Send Over LAN

A cross-platform command-line tool for transferring files directly between two machines on the same local network, over raw TCP. No cloud, no third-party server, no account — one machine listens, the other connects and sends.

Built as a hands-on project to learn C++ toolchain fundamentals (CMake, dependency management, cross-platform builds) beyond language syntax.

## Features

- Direct TCP file transfer between two machines on a LAN
- Chunked transfer with a custom lightweight wire protocol (filename + size headers, then streamed data)
- SHA-256 integrity verification — every transfer is hashed client-side during send and re-verified server-side on receipt; a failed check discards the corrupted output automatically
- Atomic writes — files are written to a temporary name and only renamed to their final name after passing verification, so a dropped connection or failed transfer never leaves a corrupted "finished" file behind
- Cross-platform: builds on Windows (MSVC) and Linux
- CI-verified builds via GitHub Actions on every push (`ubuntu-latest` and `windows-latest`)

## Requirements

- CMake 3.20+
- A C++17 compiler (MSVC on Windows, GCC on Linux)
- Ninja (recommended build generator)
- Internet access on first build — dependencies (standalone Asio, PicoSHA2) are fetched automatically via CMake's `FetchContent`, no manual installation needed

## Building

```bash
git clone https://github.com/KireinaR/solan.git
cd solan
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

On Windows, run this from a **Developer PowerShell for VS 2026** (or equivalent) so the MSVC compiler is on `PATH`.

## Usage

**Start a receiver** (on the machine that will receive the file):
```bash
./solan server
```
This starts listening on port `48562` and prints the machine's status as files arrive.

**Send a file** (from the sending machine):
```bash
./solan client <receiver-ip> <path-to-file>
```
Example:
```bash
./solan client 192.168.1.38 photo.png
```

Find the receiving machine's LAN IP with `ip addr` (Linux) or `ipconfig` (Windows).

The received file is saved as `received_<original-filename>` in the directory the server was run from.

## How it works

SOLAN uses a simple custom protocol over a single TCP connection:

1. Client connects to the server's listening socket.
2. Client sends the filename length, then the filename itself.
3. Client sends the file size (8 bytes).
4. Client streams the file in 4 KB chunks, hashing each chunk as it goes.
5. Client sends the final SHA-256 digest.
6. Server writes incoming bytes to a temporary file while computing its own hash in parallel.
7. Server compares its computed hash against the one it received. On a match, the temp file is renamed to its final name. On a mismatch, the corrupted temp file is deleted and the transfer is reported as failed.

## Project status

Currently supports direct one-to-one transfers with a manually entered IP address. 

## Planned next
1. LAN peer discovery via UDP broadcast, so machines can find each other without typing an IP manually.
2. Prompt whether to allow file transfer (on receiver end)

## Tech stack

- **Asio** (standalone, header-only) for TCP networking
- **PicoSHA2** (header-only) for SHA-256 hashing
- **CMake** with `FetchContent` for dependency management — no package manager or vendored libraries required
- **GitHub Actions** for cross-platform CI
