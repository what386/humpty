#include <sstream>
#include <string_view>

#include "arguments.hpp"

namespace humpty::cli {

std::string usage_text(std::string_view program_name) {
    std::ostringstream out;
    out << "Usage:\n"
        << "  " << program_name << " split --input|-i <file> --out|-o <dir> --chunk-size|-c <size>\n"
        << "  " << program_name << " join --manifest|-m <file> --output|-o <file> [--no-verify|-n]\n"
        << "  " << program_name << " --help\n"
        << "  " << program_name << " --version\n\n"
        << "Chunk size examples:\n"
        << "  1048576   (bytes)\n"
        << "  1M        (MiB)\n"
        << "  512K\n";
    return out.str();
}

}  // namespace humpty::cli
