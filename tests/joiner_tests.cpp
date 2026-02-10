#include "test_decls.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>

#include "services/joiner.hpp"
#include "services/splitter.hpp"
#include "test_utils.hpp"

namespace humpty::tests {

bool run_joiner_tests(std::string& error) {
    const auto temp_dir = make_temp_dir("joiner");
    const auto input_path = temp_dir / "input.bin";
    const auto output_dir = temp_dir / "chunks";
    const auto joined_path = temp_dir / "joined.bin";

    const auto input = make_test_data(130000);
    if (!write_bytes(input_path, input, error)) {
        return false;
    }

    services::SplitRequest split_request;
    split_request.input_file = input_path;
    split_request.output_dir = output_dir;
    split_request.chunk_size_bytes = 32768;

    services::SplitResult split_result;
    if (!services::split_file(split_request, split_result, error)) {
        return false;
    }

    const auto corrupt_chunk = output_dir / "input.bin.part0001";
    std::fstream chunk_file(corrupt_chunk, std::ios::in | std::ios::out | std::ios::binary);
    if (!chunk_file.is_open()) {
        error = "Failed opening chunk to corrupt: " + corrupt_chunk.string();
        return false;
    }
    char bad = static_cast<char>(0x7F);
    chunk_file.seekp(10, std::ios::beg);
    chunk_file.write(&bad, 1);
    chunk_file.close();

    services::JoinRequest join_request;
    join_request.manifest_path = split_result.manifest_path;
    join_request.output_file = joined_path;
    join_request.verify_checksums = true;

    services::JoinResult join_result;
    if (services::join_file(join_request, join_result, error)) {
        error = "Join should fail when a chunk checksum is corrupted";
        return false;
    }

    const auto found = error.find("checksum mismatch");
    if (!check(found != std::string::npos, "Expected checksum mismatch error", error)) {
        return false;
    }

    return true;
}

}  // namespace humpty::tests
