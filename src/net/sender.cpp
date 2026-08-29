#include "net/sender.h"
#include "net/protocol.h"
#include "ui/theme.h"
#include "ui/text.h"
#include "ui/spinner.h"
#include "ui/layout.h"
#include "ui/status_view.h"
#include <picosha2.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

void run_client(asio::io_context &io, const std::string &host, unsigned short port,
                const std::string &filepath, const TransferInfo &info)
{
    std::ifstream in(filepath, std::ios::binary | std::ios::ate);
    if (!in)
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    const uint64_t file_size = static_cast<uint64_t>(in.tellg());
    in.seekg(0);

    const std::string endpoint = host + ":" + std::to_string(port);
    const std::string filename = std::filesystem::path(filepath).filename().string();

    // A lone file travels under its own name; a batch is an archive, so name
    // the objects inside it instead.
    const std::string payload = info.object_count == 1 ? filename
                                                       : format_objects(info.object_count);
    const std::string sized = payload + " " + ui::help("(" + format_bytes(file_size) + ")");

    asio::ip::tcp::socket socket(io);
    {
        StatusView connecting(spin::WAITING, "Reaching " + endpoint);
        asio::ip::tcp::resolver resolver(io);
        asio::connect(socket, resolver.resolve(host, std::to_string(port)));
    }
    ui::task("Connected to " + ui::element(endpoint));

    const uint32_t name_len = static_cast<uint32_t>(filename.size());

    asio::write(socket, asio::buffer(&name_len, sizeof(name_len)));
    asio::write(socket, asio::buffer(filename));
    asio::write(socket, asio::buffer(&file_size, sizeof(file_size)));

    // The receiver is being asked whether to take this. Portal's waiting
    // spinner runs until they answer.
    unsigned char response = 0;
    {
        StatusView awaiting(spin::WAITING, "Awaiting receiver, ready to send " + sized);
        asio::read(socket, asio::buffer(&response, sizeof(response)));
    }

    if (response != TRANSFER_ACCEPT)
    {
        ui::task_fail("Rejected by the receiver. Nothing left their disk, nothing left yours.");
        return;
    }

    ui::task_ok("Receiver accepted");

    picosha2::hash256_one_by_one hasher;
    hasher.init();

    std::vector<char> buffer(CHUNK_SIZE);
    uint64_t sent = 0;

    {
        StatusView sending(spin::TRANSFER, "Sending " + sized);
        sending.track_bytes(file_size);

        while (sent < file_size)
        {
            in.read(buffer.data(), std::min(CHUNK_SIZE, static_cast<size_t>(file_size - sent)));
            const std::streamsize n = in.gcount();
            if (n <= 0) throw std::runtime_error("Unexpected end of payload while sending.");

            hasher.process(buffer.begin(), buffer.begin() + n);
            asio::write(socket, asio::buffer(buffer.data(), n));
            sent += static_cast<uint64_t>(n);
            sending.set_bytes(sent);
        }

        hasher.finish();
        std::vector<unsigned char> hash(HASH_SIZE);
        hasher.get_hash_bytes(hash.begin(), hash.end());
        asio::write(socket, asio::buffer(hash));

        sending.finish("Sent " + payload +
                       (info.compressed ? ", zipped, SHA-256 attached" : ", SHA-256 attached"));
    }
}
