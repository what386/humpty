#include "models/manifest.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <string>

namespace humpty::models {
namespace {

bool parse_u64(const std::string& token, std::uint64_t& value) {
    if (token.empty()) {
        return false;
    }

    std::uint64_t parsed = 0;
    for (const char c : token) {
        if (c < '0' || c > '9') {
            return false;
        }
        const std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
        if (parsed > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
            return false;
        }
        parsed = (parsed * 10) + digit;
    }

    value = parsed;
    return true;
}

bool parse_u32(const std::string& token, std::uint32_t& value) {
    std::uint64_t parsed = 0;
    if (!parse_u64(token, parsed) || parsed > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }
    value = static_cast<std::uint32_t>(parsed);
    return true;
}

}  // namespace

bool Manifest::is_valid() const {
    if (format_version.empty() || source_file_name.empty() || chunk_size == 0) {
        return false;
    }

    if (chunks.empty()) {
        return false;
    }

    std::set<std::uint32_t> seen_indices;
    for (const auto& chunk : chunks) {
        if (!chunk.is_valid()) {
            return false;
        }
        if (!seen_indices.insert(chunk.index).second) {
            return false;
        }
    }

    return true;
}

bool write_manifest(const Manifest& manifest, const std::filesystem::path& path, std::string& error) {
    if (!manifest.is_valid()) {
        error = "Manifest is invalid.";
        return false;
    }

    std::ofstream out(path);
    if (!out.is_open()) {
        error = "Failed to open manifest for writing: " + path.string();
        return false;
    }

    out << "version " << manifest.format_version << "\n";
    out << "source_file " << std::quoted(manifest.source_file_name) << "\n";
    out << "source_size " << manifest.source_size << "\n";
    out << "chunk_size " << manifest.chunk_size << "\n";
    out << "source_checksum " << std::quoted(manifest.source_checksum) << "\n";
    out << "chunks " << manifest.chunks.size() << "\n";

    for (const auto& chunk : manifest.chunks) {
        out << "chunk " << chunk.index << " " << chunk.offset << " " << chunk.size << " "
            << std::quoted(chunk.file_name) << " " << std::quoted(chunk.checksum) << "\n";
    }

    if (!out.good()) {
        error = "Failed while writing manifest: " + path.string();
        return false;
    }

    return true;
}

std::optional<Manifest> read_manifest(const std::filesystem::path& path, std::string& error) {
    std::ifstream in(path);
    if (!in.is_open()) {
        error = "Failed to open manifest: " + path.string();
        return std::nullopt;
    }

    Manifest manifest;
    std::size_t declared_chunks = 0;
    std::size_t parsed_chunks = 0;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "version") {
            iss >> manifest.format_version;
            continue;
        }
        if (key == "source_file") {
            iss >> std::quoted(manifest.source_file_name);
            continue;
        }
        if (key == "source_size") {
            std::string token;
            iss >> token;
            if (!parse_u64(token, manifest.source_size)) {
                error = "Invalid source_size in manifest.";
                return std::nullopt;
            }
            continue;
        }
        if (key == "chunk_size") {
            std::string token;
            iss >> token;
            if (!parse_u64(token, manifest.chunk_size)) {
                error = "Invalid chunk_size in manifest.";
                return std::nullopt;
            }
            continue;
        }
        if (key == "source_checksum") {
            iss >> std::quoted(manifest.source_checksum);
            continue;
        }
        if (key == "chunks") {
            std::string token;
            iss >> token;
            std::uint64_t count = 0;
            if (!parse_u64(token, count)) {
                error = "Invalid chunks count in manifest.";
                return std::nullopt;
            }
            declared_chunks = static_cast<std::size_t>(count);
            continue;
        }
        if (key == "chunk") {
            Chunk chunk;
            std::string index_token;
            std::string offset_token;
            std::string size_token;

            iss >> index_token >> offset_token >> size_token;
            if (!parse_u32(index_token, chunk.index) || !parse_u64(offset_token, chunk.offset) ||
                !parse_u64(size_token, chunk.size)) {
                error = "Invalid chunk numeric fields in manifest.";
                return std::nullopt;
            }

            iss >> std::quoted(chunk.file_name) >> std::quoted(chunk.checksum);
            if (!chunk.is_valid()) {
                error = "Invalid chunk entry in manifest.";
                return std::nullopt;
            }

            manifest.chunks.push_back(std::move(chunk));
            ++parsed_chunks;
            continue;
        }

        error = "Unknown manifest line key: " + key;
        return std::nullopt;
    }

    if (declared_chunks != 0 && declared_chunks != parsed_chunks) {
        error = "Manifest chunk count does not match chunk entries.";
        return std::nullopt;
    }

    if (!manifest.is_valid()) {
        error = "Manifest failed validation.";
        return std::nullopt;
    }

    return manifest;
}

}  // namespace humpty::models
