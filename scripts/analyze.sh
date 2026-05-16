#!/bin/bash
set -euo pipefail

./scripts/build.sh

# Requires cppcheck and clang-tidy to be installed in the container.
# Analyze only production sources.
mapfile -t cpp_sources < <(find src -type f -name "*.cpp")
mapfile -t header_sources < <(find include -type f \( -name "*.hpp" -o -name "*.h" \))

cppcheck -j "$(nproc)" --enable=all --std=c++20 \
	-I include \
	--suppressions-list=.cppcheck-suppressions \
	"${cpp_sources[@]}" "${header_sources[@]}"

# Use the exported compile_commands.json (-p build) for a true project approach, and run in parallel.
find src -name "*.cpp" | xargs -P "$(nproc)" -I {} clang-tidy -p build {}
