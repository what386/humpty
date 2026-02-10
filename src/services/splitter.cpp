#include "services/splitter.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <span>

#include "models/chunk.hpp"
#include "models/manifest.hpp"
#include "services/checksums.hpp"

namespace humpty::services {
namespace {

constexpr std::size_t kBufferSize = 64 * 1024;

}  // namespace

bool split_file(const SplitRequest& request, SplitResult& result, std::string& error) {
    result = {};
    error.clear();

    if (request.chunk_size_bytes == 0) {
        error = "Chunk size must be greater than zero.";
        return false;
    }

    if (!std::filesystem::exists(request.input_file)) {
        error = "Input file does not exist: " + request.input_file.string();
        return false;
    }
    if (!std::filesystem::is_regular_file(request.input_file)) {
        error = "Input path is not a regular file: " + request.input_file.string();
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(request.output_dir, ec);
    if (ec) {
        error = "Failed to create output directory: " + request.output_dir.string();
        return false;
    }

    std::ifstream input(request.input_file, std::ios::binary);
    if (!input.is_open()) {
        error = "Failed to open input file: " + request.input_file.string();
        return false;
    }

    const std::uint64_t source_size = std::filesystem::file_size(request.input_file, ec);
    if (ec) {
        error = "Failed to read input size: " + request.input_file.string();
        return false;
    }

    humpty::models::Manifest manifest;
    manifest.source_file_name = request.input_file.filename().string();
    manifest.source_size = source_size;
    manifest.chunk_size = request.chunk_size_bytes;

    std::array<std::byte, kBufferSize> buffer{};
    std::uint64_t source_hash_state = 14695981039346656037ULL;
    std::uint64_t offset = 0;
    std::uint32_t chunk_index = 0;

    while (offset < source_size) {
        const std::uint64_t remaining = source_size - offset;
        const std::uint64_t chunk_size = (remaining < request.chunk_size_bytes) ? remaining : request.chunk_size_bytes;

        humpty::models::Chunk chunk;
        chunk.index = chunk_index;
        chunk.offset = offset;
        chunk.size = chunk_size;
        chunk.file_name = humpty::models::make_chunk_filename(manifest.source_file_name, chunk_index);

        const std::filesystem::path chunk_path = request.output_dir / chunk.file_name;
        std::ofstream chunk_out(chunk_path, std::ios::binary);
        if (!chunk_out.is_open()) {
            error = "Failed to open chunk for writing: " + chunk_path.string();
            return false;
        }

        std::uint64_t chunk_bytes_written = 0;
        std::uint64_t chunk_hash_state = 14695981039346656037ULL;

        while (chunk_bytes_written < chunk_size) {
            const std::uint64_t chunk_remaining = chunk_size - chunk_bytes_written;
            const std::size_t to_read = static_cast<std::size_t>(
                (chunk_remaining < static_cast<std::uint64_t>(buffer.size())) ? chunk_remaining : buffer.size());

            input.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(to_read));
            const std::streamsize got = input.gcount();
            if (got <= 0) {
                error = "Unexpected end of input while splitting file.";
                return false;
            }

            const auto view = std::span<const std::byte>(buffer.data(), static_cast<std::size_t>(got));
            chunk_out.write(reinterpret_cast<const char*>(buffer.data()), got);
            if (!chunk_out.good()) {
                error = "Failed writing chunk file: " + chunk_path.string();
                return false;
            }

            chunk_hash_state = fnv1a64(view, chunk_hash_state);
            source_hash_state = fnv1a64(view, source_hash_state);
            chunk_bytes_written += static_cast<std::uint64_t>(got);
            offset += static_cast<std::uint64_t>(got);
        }

        chunk.checksum = fnv1a64_hex({}, chunk_hash_state);
        manifest.chunks.push_back(std::move(chunk));
        ++chunk_index;
    }

    manifest.source_checksum = fnv1a64_hex({}, source_hash_state);
    if (!manifest.is_valid()) {
        error = "Generated manifest is invalid.";
        return false;
    }

    const auto manifest_path = request.output_dir / (manifest.source_file_name + ".manifest");
    if (!humpty::models::write_manifest(manifest, manifest_path, error)) {
        return false;
    }

    result.manifest_path = manifest_path;
    result.chunk_count = manifest.chunks.size();
    result.total_bytes = manifest.source_size;
    return true;
}

}  // namespace humpty::services
