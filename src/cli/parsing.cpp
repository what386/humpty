#include <cctype>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>

#include "parsing.hpp"

namespace humpty::cli {
namespace {

bool parse_size_bytes(std::string_view raw, std::uint64_t& out_size) {
    if (raw.empty()) {
        return false;
    }

    std::uint64_t multiplier = 1;
    char suffix = static_cast<char>(std::toupper(static_cast<unsigned char>(raw.back())));
    if (suffix == 'K' || suffix == 'M' || suffix == 'G') {
        raw.remove_suffix(1);
        if (raw.empty()) {
            return false;
        }
        if (suffix == 'K') {
            multiplier = 1024ULL;
        } else if (suffix == 'M') {
            multiplier = 1024ULL * 1024ULL;
        } else {
            multiplier = 1024ULL * 1024ULL * 1024ULL;
        }
    }

    std::uint64_t value = 0;
    for (char c : raw) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
        const std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
        if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
            return false;
        }
        value = (value * 10) + digit;
    }

    if (value == 0 || value > std::numeric_limits<std::uint64_t>::max() / multiplier) {
        return false;
    }

    out_size = value * multiplier;
    return true;
}

}  // namespace

ParsedArgs parse_arguments(int argc, char* argv[]) {
    ParsedArgs parsed;

    if (argc <= 1) {
        parsed.command = CommandType::Help;
        parsed.show_usage = true;
        return parsed;
    }

    const std::string_view command = argv[1];

    if (command == "-h" || command == "--help") {
        parsed.command = CommandType::Help;
        parsed.show_usage = true;
        return parsed;
    }

    if (command == "-v" || command == "--version") {
        parsed.command = CommandType::Version;
        return parsed;
    }

    if (command == "split") {
        SplitArgs split;
        bool saw_input_positional = false;

        for (int i = 2; i < argc; ++i) {
            const std::string_view token = argv[i];
            if (token == "--help" || token == "-h") {
                parsed.command = CommandType::Help;
                parsed.show_usage = true;
                return parsed;
            }

            if ((token == "--input" || token == "-i") && (i + 1) < argc) {
                split.input_path = argv[++i];
                continue;
            }
            if ((token == "--out" || token == "-o") && (i + 1) < argc) {
                split.output_dir = argv[++i];
                continue;
            }
            if ((token == "--chunk-size" || token == "-c") && (i + 1) < argc) {
                std::uint64_t size = 0;
                if (!parse_size_bytes(argv[++i], size)) {
                    parsed.command = CommandType::Invalid;
                    parsed.error = "Invalid --chunk-size/-c. Use positive integer bytes, optionally with K/M/G suffix.";
                    return parsed;
                }
                split.chunk_size_bytes = size;
                continue;
            }
            if (!token.empty() && token.front() != '-' && !saw_input_positional) {
                split.input_path = std::string(token);
                saw_input_positional = true;
                continue;
            }

            parsed.command = CommandType::Invalid;
            parsed.error = "Unknown or incomplete split argument: " + std::string(token);
            return parsed;
        }

        if (split.input_path.empty() || split.chunk_size_bytes == 0) {
            parsed.command = CommandType::Invalid;
            parsed.error = "split requires <input-file> and --chunk-size/-c <bytes|K|M|G>.";
            return parsed;
        }
        if (split.output_dir.empty()) {
            const auto file_name = std::filesystem::path(split.input_path).filename().string();
            split.output_dir = "./" + file_name + "-humpty";
        }

        parsed.command = CommandType::Split;
        parsed.split = split;
        return parsed;
    }

    if (command == "join") {
        JoinArgs join;
        bool saw_manifest_positional = false;

        for (int i = 2; i < argc; ++i) {
            const std::string_view token = argv[i];
            if (token == "--help" || token == "-h") {
                parsed.command = CommandType::Help;
                parsed.show_usage = true;
                return parsed;
            }
            if ((token == "--manifest" || token == "-m") && (i + 1) < argc) {
                join.manifest_path = argv[++i];
                continue;
            }
            if ((token == "--output" || token == "-o") && (i + 1) < argc) {
                join.output_path = argv[++i];
                continue;
            }
            if (token == "--no-verify" || token == "-n") {
                join.verify_checksums = false;
                continue;
            }
            if (!token.empty() && token.front() != '-' && !saw_manifest_positional) {
                join.manifest_path = std::string(token);
                saw_manifest_positional = true;
                continue;
            }

            parsed.command = CommandType::Invalid;
            parsed.error = "Unknown or incomplete join argument: " + std::string(token);
            return parsed;
        }

        if (join.manifest_path.empty() || join.output_path.empty()) {
            parsed.command = CommandType::Invalid;
            parsed.error = "join requires <manifest-file> and --output/-o <path>.";
            return parsed;
        }

        parsed.command = CommandType::Join;
        parsed.join = join;
        return parsed;
    }

    parsed.command = CommandType::Invalid;
    parsed.error = "Unknown command: " + std::string(command);
    return parsed;
}

}  // namespace humpty::cli
