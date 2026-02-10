#include <iostream>

#include "dispatch.hpp"
#include "arguments.hpp"
#include "parsing.hpp"
#include "services/joiner.hpp"
#include "services/splitter.hpp"

namespace humpty::cli {
namespace {

constexpr const char* kVersion = "0.1.0";

int dispatch_split(const SplitArgs& args) {
    services::SplitRequest request;
    request.input_file = args.input_path;
    request.output_dir = args.output_dir;
    request.chunk_size_bytes = args.chunk_size_bytes;

    services::SplitResult result;
    std::string error;
    if (!services::split_file(request, result, error)) {
        std::cerr << "split failed: " << error << "\n";
        return 2;
    }

    std::cout << "split complete\n"
              << "manifest: " << result.manifest_path.string() << "\n"
              << "chunks: " << result.chunk_count << "\n"
              << "bytes: " << result.total_bytes << "\n";
    return 0;
}

int dispatch_join(const JoinArgs& args) {
    services::JoinRequest request;
    request.manifest_path = args.manifest_path;
    request.output_file = args.output_path;
    request.verify_checksums = args.verify_checksums;

    services::JoinResult result;
    std::string error;
    if (!services::join_file(request, result, error)) {
        std::cerr << "join failed: " << error << "\n";
        return 2;
    }

    std::cout << "join complete\n"
              << "bytes: " << result.total_bytes_written << "\n";
    return 0;
}

}  // namespace

int run(int argc, char* argv[]) {
    const ParsedArgs parsed = parse_arguments(argc, argv);

    switch (parsed.command) {
    case CommandType::Help:
        std::cout << usage_text(argc > 0 ? argv[0] : "humpty");
        return 0;
    case CommandType::Version:
        std::cout << "humpty " << kVersion << "\n";
        return 0;
    case CommandType::Split:
        return dispatch_split(*parsed.split);
    case CommandType::Join:
        return dispatch_join(*parsed.join);
    case CommandType::Invalid:
        std::cerr << "Error: " << parsed.error << "\n\n"
                  << usage_text(argc > 0 ? argv[0] : "humpty");
        return 1;
    }

    std::cerr << "Error: unexpected command\n";
    return 1;
}

}  // namespace humpty::cli
