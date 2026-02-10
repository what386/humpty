#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build
cmake --build build --target humpty_tests
ctest --test-dir build --output-on-failure -R "^humpty_tests$"
