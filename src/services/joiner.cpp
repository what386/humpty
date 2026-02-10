#include "services/joiner.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <span>

#include "models/manifest.hpp"
#include "services/checksums.hpp"

namespace humpty::services {
namespace {

constexpr std::size_t kBufferSize = 64 * 1024;

std::string hash_state_to_hex(std::uint64_t state) {
    return fnv1a64_hex({}, state);
}

}  // namespace

bool join_file(const JoinRequest& request, JoinResult& result, std::string& error) {
    result = {};
    error.clear();

    std::string manifest_error;
    const auto manifest_opt = humpty::models::read_manifest(request.manifest_path, manifest_error);
    if (!manifest_opt.has_value()) {
        error = manifest_error;
        return false;
    }

    const humpty::models::Manifest& manifest = *manifest_opt;
    const auto base_dir = request.manifest_path.parent_path();

    std::ofstream output(request.output_file, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        error = "Failed to open output file: " + request.output_file.string();
        return false;
    }

    std::array<std::byte, kBufferSize> buffer{};
    std::uint64_t source_hash_state = 14695981039346656037ULL;
    std::uint64_t total_bytes_written = 0;

    for (const auto& chunk : manifest.chunks) {
        const auto chunk_path = base_dir / chunk.file_name;
        std::ifstream chunk_in(chunk_path, std::ios::binary);
        if (!chunk_in.is_open()) {
            error = "Failed to open chunk file: " + chunk_path.string();
            return false;
        }

        std::uint64_t chunk_hash_state = 14695981039346656037ULL;
        std::uint64_t chunk_bytes_read = 0;

        while (chunk_bytes_read < chunk.size) {
            const std::uint64_t chunk_remaining = chunk.size - chunk_bytes_read;
            const std::size_t to_read = static_cast<std::size_t>(
                (chunk_remaining < static_cast<std::uint64_t>(buffer.size())) ? chunk_remaining : buffer.size());

            chunk_in.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(to_read));
            const std::streamsize got = chunk_in.gcount();
            if (got <= 0) {
                error = "Unexpected end of chunk: " + chunk_path.string();
                return false;
            }

            output.write(reinterpret_cast<const char*>(buffer.data()), got);
            if (!output.good()) {
                error = "Failed writing output file: " + request.output_file.string();
                return false;
            }

            const auto view = std::span<const std::byte>(buffer.data(), static_cast<std::size_t>(got));
            chunk_hash_state = fnv1a64(view, chunk_hash_state);
            source_hash_state = fnv1a64(view, source_hash_state);
            chunk_bytes_read += static_cast<std::uint64_t>(got);
            total_bytes_written += static_cast<std::uint64_t>(got);
        }

        if (request.verify_checksums && !chunk.checksum.empty()) {
            const auto actual = hash_state_to_hex(chunk_hash_state);
            if (actual != chunk.checksum) {
                error = "Chunk checksum mismatch for " + chunk.file_name;
                return false;
            }
        }
    }

    if (request.verify_checksums && !manifest.source_checksum.empty()) {
        const auto actual = hash_state_to_hex(source_hash_state);
        if (actual != manifest.source_checksum) {
            error = "Source checksum mismatch after join.";
            return false;
        }
    }

    if (manifest.source_size != 0 && total_bytes_written != manifest.source_size) {
        error = "Output size does not match manifest source_size.";
        return false;
    }

    result.total_bytes_written = total_bytes_written;
    return true;
}

}  // namespace humpty::services
