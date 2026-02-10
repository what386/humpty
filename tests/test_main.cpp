#include <iostream>
#include <string>
#include <string_view>

#include "test_decls.hpp"

namespace {

bool run_case(std::string_view name, std::string& error) {
    if (name == "splitter") {
        return humpty::tests::run_splitter_tests(error);
    }
    if (name == "joiner") {
        return humpty::tests::run_joiner_tests(error);
    }
    if (name == "roundtrip") {
        return humpty::tests::run_roundtrip_tests(error);
    }
    if (name == "urandom") {
        return humpty::tests::run_urandom_tests(error);
    }
    if (name == "all") {
        return humpty::tests::run_splitter_tests(error) && humpty::tests::run_joiner_tests(error) &&
               humpty::tests::run_roundtrip_tests(error) && humpty::tests::run_urandom_tests(error);
    }
    error = "Unknown test case: " + std::string(name);
    return false;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string error;
    std::string case_name = "all";

    for (int i = 1; i < argc; ++i) {
        const std::string_view token = argv[i];
        if ((token == "--case" || token == "-c") && (i + 1) < argc) {
            case_name = argv[++i];
            continue;
        }
        if (token == "--help" || token == "-h") {
            std::cout << "Usage: humpty_tests [--case|-c <all|splitter|joiner|roundtrip|urandom>]\n";
            return 0;
        }
        std::cerr << "Unknown argument: " << token << "\n";
        return 1;
    }

    if (!run_case(case_name, error)) {
        std::cerr << case_name << "_tests failed: " << error << "\n";
        return 1;
    }

    std::cout << case_name << "_tests passed\n";
    return 0;
}
