#!/bin/bash
# Execute find and sed to trim trailing whitespace
find . -type f -name "*.cpp" -exec sed -i 's/[[:space:]]*$//' {} +
