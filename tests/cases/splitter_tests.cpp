#include "test_decls.hpp"

#include <filesystem>
#include <string>

#include "models/manifest.hpp"
#include "services/splitter.hpp"
#include "test_utils.hpp"

namespace humpty::tests {

bool run_splitter_tests(std::string& error) {
    const auto temp_dir = make_temp_dir("splitter");
    const auto input_path = temp_dir / "input.bin";
    const auto output_dir = temp_dir / "chunks";

    const auto input = make_test_data(200000);
    if (!write_bytes(input_path, input, error)) {
        return false;
    }

    services::SplitRequest request;
    request.input_file = input_path;
    request.output_dir = output_dir;
    request.chunk_size_bytes = 65536;

    services::SplitResult result;
    if (!services::split_file(request, result, error)) {
        return false;
    }

    if (!check(std::filesystem::exists(result.manifest_path), "Manifest path missing", error)) {
        return false;
    }
    if (!check(result.chunk_count == 4, "Expected 4 chunks for 200000 bytes at 65536 chunk size", error)) {
        return false;
    }
    if (!check(result.total_bytes == input.size(), "Split total bytes mismatch", error)) {
        return false;
    }

    std::string manifest_error;
    const auto manifest = models::read_manifest(result.manifest_path, manifest_error);
    if (!manifest.has_value()) {
        error = "Failed reading manifest: " + manifest_error;
        return false;
    }

    if (!check(manifest->chunks.size() == result.chunk_count, "Manifest chunk count mismatch", error)) {
        return false;
    }

    for (const auto& chunk : manifest->chunks) {
        const auto path = output_dir / chunk.file_name;
        if (!check(std::filesystem::exists(path), "Missing chunk file: " + path.string(), error)) {
            return false;
        }
    }

    return true;
}

}  // namespace humpty::tests
