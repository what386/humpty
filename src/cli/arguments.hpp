#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace humpty::cli {

enum class CommandType {
    Help,
    Version,
    Split,
    Join,
    Invalid,
};

struct SplitArgs {
    std::string input_path;
    std::string output_dir;
    std::uint64_t chunk_size_bytes = 0;
};

struct JoinArgs {
    std::string manifest_path;
    std::string output_path;
    bool verify_checksums = true;
};

struct ParsedArgs {
    CommandType command = CommandType::Invalid;
    std::optional<SplitArgs> split;
    std::optional<JoinArgs> join;
    std::string error;
    bool show_usage = false;
};

std::string usage_text(std::string_view program_name);

}  // namespace humpty::cli
