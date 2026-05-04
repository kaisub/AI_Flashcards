#!/bin/bash
# Generate Ninja build files (-G Ninja) which are significantly faster than Makefiles
cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel