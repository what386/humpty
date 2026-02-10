#include "test_decls.hpp"

#include <string>
#include <vector>

#include "services/joiner.hpp"
#include "services/splitter.hpp"
#include "test_utils.hpp"

namespace humpty::tests {

bool run_roundtrip_tests(std::string& error) {
    const auto temp_dir = make_temp_dir("roundtrip");
    const auto input_path = temp_dir / "input.bin";
    const auto output_dir = temp_dir / "chunks";
    const auto joined_path = temp_dir / "joined.bin";

    const auto input = make_test_data(400000);
    if (!write_bytes(input_path, input, error)) {
        return false;
    }

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

    std::vector<std::byte> output;
    if (!read_bytes(joined_path, output, error)) {
        return false;
    }

    if (!check(output == input, "Joined output does not match original input", error)) {
        return false;
    }
    if (!check(join_result.total_bytes_written == input.size(), "Joined byte count mismatch", error)) {
        return false;
    }

    return true;
}

}  // namespace humpty::tests
