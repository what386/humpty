#include "services/checksums.hpp"

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace humpty::services {
namespace {

constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;
constexpr std::size_t kBufferSize = 64 * 1024;

std::string to_hex64(std::uint64_t value) {
    std::ostringstream out;
    out << std::hex << std::nouppercase << std::setfill('0') << std::setw(16) << value;
    return out.str();
}

}  // namespace

std::uint64_t fnv1a64(std::span<const std::byte> data, std::uint64_t seed) {
    std::uint64_t hash = seed;
    for (const std::byte b : data) {
        hash ^= static_cast<std::uint64_t>(b);
        hash *= kFnvPrime;
    }
    return hash;
}

std::string fnv1a64_hex(std::span<const std::byte> data, std::uint64_t seed) {
    return to_hex64(fnv1a64(data, seed));
}

bool hash_file_fnv1a64_hex(const std::filesystem::path& path, std::string& out_hex, std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        error = "Failed to open file for hashing: " + path.string();
        return false;
    }

    std::array<std::byte, kBufferSize> buffer{};
    std::uint64_t hash = kFnvOffsetBasis;

    while (in.good()) {
        in.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize bytes_read = in.gcount();
        if (bytes_read <= 0) {
            break;
        }

        const auto view = std::span<const std::byte>(buffer.data(), static_cast<std::size_t>(bytes_read));
        hash = fnv1a64(view, hash);
    }

    if (!in.eof() && in.fail()) {
        error = "Failed while hashing file: " + path.string();
        return false;
    }

    out_hex = to_hex64(hash);
    return true;
}

}  // namespace humpty::services
