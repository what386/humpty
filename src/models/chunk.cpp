#include "models/chunk.hpp"

#include <iomanip>
#include <sstream>

namespace humpty::models {

bool Chunk::is_valid() const {
    return !file_name.empty();
}

std::string make_chunk_filename(std::string_view base_name, std::uint32_t index, unsigned int width) {
    std::ostringstream out;
    out << base_name << ".part" << std::setw(static_cast<int>(width)) << std::setfill('0') << index;
    return out.str();
}

}  // namespace humpty::models
