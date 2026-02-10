#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace humpty::services {

struct JoinRequest {
    std::filesystem::path manifest_path;
    std::filesystem::path output_file;
    bool verify_checksums = true;
};

struct JoinResult {
    std::uint64_t total_bytes_written = 0;
};

bool join_file(const JoinRequest& request, JoinResult& result, std::string& error);

}  // namespace humpty::services
