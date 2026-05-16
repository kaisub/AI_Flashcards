# AI Flashcards

Terminal flashcards application written in C++20 with FTXUI.

The project supports:
- browsing flashcard lists and folders
- creating and editing decks
- running study sessions with multiple card states
- JSON-based storage
- compile-time UI localization

## Tech Stack

- C++20
- CMake 3.22+
- Ninja
- FTXUI
- nlohmann/json
- GoogleTest

Dependencies are fetched automatically by CMake via `FetchContent`.

## Project Layout

- `src/` application and view implementation
- `include/` public headers
- `tests/` unit tests
- `scripts/` helper scripts for build, test, and run
- root helper utilities such as `into_container.sh` and `context_merger.py`
- `data/` example flashcard data
- `include/app/localization/` language catalogs and locale selector

## Build

Configure and build with the default Polish locale:

```bash
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
```

Build with a specific UI locale:

```bash
cmake -S . -B build -G Ninja -DAI_FLASHCARDS_LOCALE=pl
cmake --build build --parallel
```

```bash
cmake -S . -B build -G Ninja -DAI_FLASHCARDS_LOCALE=en
cmake --build build --parallel
```

Supported locale values:
- `pl`
- `en`

## Run

Run the application after building:

```bash
./build/flashcards_app
```

Or use the helper script:

```bash
./scripts/run.sh
```

## Test

Run the full test suite with CTest:

```bash
ctest --test-dir build --output-on-failure
```

Or use the helper script:

```bash
./scripts/build_and_test.sh
```

## Helper Scripts

- `./scripts/build.sh` configures and builds the project
- `./scripts/build_and_test.sh` builds and runs tests
- `./scripts/run.sh` builds, tests, and launches the app
- `./scripts/analyze.sh` builds the project and runs `cppcheck` plus `clang-tidy`
- `./scripts/clean.sh` removes build artifacts
- `./scripts/trim_trailing.sh` removes trailing whitespace from `.cpp` files

Notes for helper scripts:

- `./scripts/analyze.sh` requires `cppcheck` and `clang-tidy` to be installed
- `./scripts/clean.sh` removes `build/` and `bin/`

## Root Utilities

- `./into_container.sh` finds the running project dev container and opens an interactive shell in it
- `./context_merger.py` generates `project_context.txt` by merging selected project files into one text file

## Localization

UI strings are organized in:

- `include/app/localization/LocalizedText.hpp`
- `include/app/localization/LocalizationData.hpp`
- `include/app/localization/Localization.hpp`

`LocalizationData.hpp` holds both languages in one table, while `Localization.hpp` keeps the runtime locale selector used by the UI.

To add another language:

1. Extend `include/app/localization/LocalizationData.hpp` with the new translations.
2. Update `include/app/localization/Localization.hpp` if the locale enum or default selection changes.
3. Adjust the UI if you want a new language switch surface.

## Build Outputs

Main binaries:

- `build/flashcards_app`
- `build/flashcards_tests`

## Notes

- The build currently uses Ninja in the helper scripts.
- `compile_commands.json` is generated in `build/` for editor tooling.
- The app stores flashcard data as JSON files.