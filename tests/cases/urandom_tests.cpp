#include "test_decls.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <string>

#include "services/checksums.hpp"
#include "services/joiner.hpp"
#include "services/splitter.hpp"
#include "test_utils.hpp"

namespace humpty::tests {

bool run_urandom_tests(std::string& error) {
    const std::filesystem::path random_path = "/dev/urandom";
    if (!std::filesystem::exists(random_path)) {
        return true;
    }

    std::ifstream random_in(random_path, std::ios::binary);
    if (!random_in.is_open()) {
        return true;
    }

    const auto temp_dir = make_temp_dir("urandom");
    const auto input_path = temp_dir / "input.bin";
    const auto output_dir = temp_dir / "chunks";
    const auto joined_path = temp_dir / "joined.bin";

    std::ofstream input_out(input_path, std::ios::binary | std::ios::trunc);
    if (!input_out.is_open()) {
        error = "Failed to create urandom input file.";
        return false;
    }

    constexpr std::size_t kBytesToGenerate = 512 * 1024;
    std::array<char, 8192> buffer{};
    std::size_t total = 0;
    while (total < kBytesToGenerate) {
        const std::size_t remaining = kBytesToGenerate - total;
        const std::size_t want = (remaining < buffer.size()) ? remaining : buffer.size();

        random_in.read(buffer.data(), static_cast<std::streamsize>(want));
        const auto got = random_in.gcount();
        if (got <= 0) {
            error = "Failed to read bytes from /dev/urandom.";
            return false;
        }

        input_out.write(buffer.data(), got);
        if (!input_out.good()) {
            error = "Failed to write generated urandom input.";
            return false;
        }

        total += static_cast<std::size_t>(got);
    }

    input_out.close();

    services::SplitRequest split_request;
    split_request.input_file = input_path;
    split_request.output_dir = output_dir;
    split_request.chunk_size_bytes = 65536;

    services::SplitResult split_result;
    if (!services::split_file(split_request, split_result, error)) {
        return false;
    }

    services::JoinRequest join_request;
    join_request.manifest_path = split_result.manifest_path;
    join_request.output_file = joined_path;
    join_request.verify_checksums = true;

    services::JoinResult join_result;
    if (!services::join_file(join_request, join_result, error)) {
        return false;
    }

    std::string input_hash;
    if (!services::hash_file_fnv1a64_hex(input_path, input_hash, error)) {
        return false;
    }

    std::string joined_hash;
    if (!services::hash_file_fnv1a64_hex(joined_path, joined_hash, error)) {
        return false;
    }

    if (!check(input_hash == joined_hash, "Joined urandom file hash mismatch", error)) {
        return false;
    }
    if (!check(join_result.total_bytes_written == kBytesToGenerate, "Joined urandom byte count mismatch", error)) {
        return false;
    }

    return true;
}

}  // namespace humpty::tests
