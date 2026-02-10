#include <iostream>
#include <string>

#include "test_decls.hpp"

int main() {
    std::string error;

    if (!humpty::tests::run_splitter_tests(error)) {
        std::cerr << "splitter_tests failed: " << error << "\n";
        return 1;
    }
    if (!humpty::tests::run_joiner_tests(error)) {
        std::cerr << "joiner_tests failed: " << error << "\n";
        return 1;
    }
    if (!humpty::tests::run_roundtrip_tests(error)) {
        std::cerr << "roundtrip_tests failed: " << error << "\n";
        return 1;
    }
    if (!humpty::tests::run_urandom_tests(error)) {
        std::cerr << "urandom_tests failed: " << error << "\n";
        return 1;
    }

    std::cout << "All tests passed\n";
    return 0;
}
