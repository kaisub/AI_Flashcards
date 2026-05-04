#!/bin/bash
./scripts/build.sh
# Requires cppcheck and clang-tidy to be installed in the container
cppcheck -j $(nproc) --enable=all --std=c++20 -I include -I tests/app src/*.cpp include/core/*.hpp tests/*.cpp --suppressions-list=.cppcheck-suppressions
# Use the exported compile_commands.json (-p build) for a true project approach, and run in parallel
find src -name "*.cpp" | xargs -P $(nproc) -I {} clang-tidy -p build {}
