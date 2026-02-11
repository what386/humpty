#!/usr/bin/env bash
set -euo pipefail

xmake build humpty_tests
xmake run humpty_tests --case all
xmake run humpty_tests --case splitter
xmake run humpty_tests --case joiner
xmake run humpty_tests --case roundtrip
xmake run humpty_tests --case urandom
