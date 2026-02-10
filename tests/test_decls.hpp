#pragma once

#include <string>

namespace humpty::tests {

bool run_splitter_tests(std::string& error);
bool run_joiner_tests(std::string& error);
bool run_roundtrip_tests(std::string& error);

}  // namespace humpty::tests
