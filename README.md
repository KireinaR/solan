# SOLAN, Send Over LAN

A command line tool for sending files directly between two computers on the same local network. No cloud storage, no third party server, no account needed. One computer listens, the other one connects and sends. That's it.

This project started as a way to actually learn C++ properly. Not competitive programming or algorithm puzzles, but the real, practical stuff that tutorials usually skip: how to set up CMake, how to manage third party libraries without a package manager, how to get a project building the same way on both Windows and Linux, and how to structure a codebase so it doesn't fall apart as it grows.

## What it can do

**Direct file transfer over TCP**
Send a file straight from one machine to another. No middleman, no upload step.

**Automatic peer discovery**
Finds other SOLAN instances on the network via UDP broadcast. No need to type IP addresses by hand.

**Interactive shell**
Run `solan` with no arguments for a small interactive shell, similar in spirit to the MongoDB shell.

**Integrity verification**
Every file is hashed with SHA-256 on send and re-checked on arrival. A mismatch gets discarded automatically.

**Safe writes**
Incoming files write to a temporary file first, renamed only after the hash check passes. No broken partial files left behind.

**Accept or reject transfers**
You get a prompt before any file bytes are transferred, not after.

**Persistent receive mode**
The receiver keeps listening after each file, handling transfer after transfer in one session.

**Live progress bar**
Both sending and receiving show progress as it happens.

**Cross platform**
Builds and runs on Windows and Linux, verified on every push via GitHub Actions on both `ubuntu-latest` and `windows-latest`.

**Multi-file transfers**
Send more than one file at a time. SOLAN packs them into a single zip archive before sending.

## Requirements

- CMake 3.20 or newer
- A C++17 compiler (MSVC on Windows, GCC on Linux)
- Ninja is recommended as the build generator
- An internet connection the first time you build, since dependencies (standalone Asio and PicoSHA2) are downloaded automatically through CMake's `FetchContent`. You don't need to install them yourself.

## Building it

```bash
git clone https://github.com/<your-username>/solan.git
cd solan
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

On Windows, run these commands from a "Developer PowerShell for VS 2026" (or whichever Visual Studio version you have) so the compiler is properly set up on your PATH.

## Using it

### The interactive shell

Just run the executable with no arguments:

```bash
./solan
```

You'll land in a small shell that looks something like this:

```
SOLAN v0.1.0 - Send Over LAN
Type "help" for commands.

solan >>
```

From here, `help` shows you the available commands at any point, and `sm <mode>` switches between the two modes:

- `sm receive` puts you in receive mode, where SOLAN starts listening for incoming files and broadcasting its presence on the network so other SOLAN instances can find it.
- `sm send` puts you in send mode, where you can discover peers and send them files.

**A typical receiving session:**

```
solan >> sm receive

Receive mode, listening on port 48562
  Ctrl+C   force-quit while idle
  exmo     exit receive mode (available after each transfer)

Connected: 192.168.1.39
Incoming: 'photo.png' (2.3 MB)
Accept? [y/n]: y
Receiving [========================================] 100.0%
Received 2415392 bytes. Verified and saved as 'received_photo.png'.
Press Enter to keep receiving, or type 'exmo' to exit:
```

**A typical sending session:**

```
solan >> sm send

Send mode. Type "help" for commands, "back" to return.

solan:send >> discover
Scanning (5s)...

Found 1 peer(s):
  [1] 192.168.1.12  (DESKTOP-JAKE)

solan:send >> send 1 photo.png
Accepted. Sending...
Sending [========================================] 100.0%
Sent 2415392 bytes.

solan:send >> back
solan >> exit
bye.
```

### Direct command line mode

If you'd rather script it or skip the interactive shell entirely, SOLAN also accepts arguments directly:

```bash
# Start a receiver on the default port
./solan server

# Send a file to a specific IP address
./solan client <ip-address> <path-to-file>
```

This is the same underlying logic as the shell, just without the menu.

## How the transfer actually works

SOLAN uses a small custom protocol over a single TCP connection. Roughly, it goes like this:

1. The client connects to the server.
2. The client sends the length of the filename, then the filename itself.
3. The client sends the file size, as an 8 byte number.
4. The server reads all of that, then asks the person running it whether to accept the transfer, and sends back a single byte saying yes or no.
5. If accepted, the client streams the file in small chunks, hashing each chunk with SHA-256 as it goes, and the server writes those chunks to a temporary file while computing its own hash in parallel.
6. Once the file is fully sent, the client sends its final hash.
7. The server compares the two hashes. If they match, the temporary file is renamed to its final name. If they don't, the temporary file is deleted and the transfer is reported as failed.

Peer discovery works separately, over UDP. A machine in receive mode periodically broadcasts a small message containing its hostname and port. Machines in send mode listen for these broadcasts for a few seconds and build a list of who responded.

## Project layout

```
solan/
├── src/
│   ├── main.cpp             entry point, dispatches to the shell or direct CLI mode
│   ├── shell.h/.cpp          the interactive shell (REPL)
│   ├── net/
│   │   ├── protocol.h        shared constants (ports, chunk size, etc.)
│   │   ├── sender.h/.cpp      client side logic
│   │   ├── receiver.h/.cpp    server side logic
│   │   └── discovery.h/.cpp   UDP broadcast and peer discovery
│   └── ui/
│       └── progress.h/.cpp    the progress bar
├── CMakeLists.txt
└── .github/workflows/        GitHub Actions CI config
```

## Tech stack

- **Asio** (standalone, header only) for the networking layer
- **PicoSHA2** (header only) for SHA-256 hashing
- **CMake** with `FetchContent` for dependency management, so there's nothing to install manually
- **GitHub Actions** for continuous cross platform build verification

## What's next

Right now, peer discovery gives you an IP address and a hostname. A natural next step would be showing the MAC address of discovered peers too, though that turns out to be more involved than it sounds since it means reading the operating system's own ARP table rather than anything available directly over the socket.

An installer for Windows is also on the list, one that would set up the app and add it to your PATH automatically, so you can just type `solan` from any terminal without building it yourself.

## License

MIT