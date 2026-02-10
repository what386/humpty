#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace humpty::services {

struct SplitRequest {
    std::filesystem::path input_file;
    std::filesystem::path output_dir;
    std::uint64_t chunk_size_bytes = 0;
};

struct SplitResult {
    std::filesystem::path manifest_path;
    std::size_t chunk_count = 0;
    std::uint64_t total_bytes = 0;
};

bool split_file(const SplitRequest& request, SplitResult& result, std::string& error);

}  // namespace humpty::services
