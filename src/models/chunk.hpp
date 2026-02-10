#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace humpty::models {

struct Chunk {
    std::uint32_t index = 0;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
    std::string file_name;
    std::string checksum;

    [[nodiscard]] bool is_valid() const;
};

std::string make_chunk_filename(std::string_view base_name, std::uint32_t index, unsigned int width = 4);

}  // namespace humpty::models
