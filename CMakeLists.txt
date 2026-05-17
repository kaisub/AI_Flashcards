# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)

# Project definition
project(AIFlashcards VERSION 0.1.0 LANGUAGES CXX)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Disable compiler-specific extensions (e.g., gnu++20)

set(AI_FLASHCARDS_LOCALE "pl" CACHE STRING "UI locale to compile into the app (pl or en)")
set_property(CACHE AI_FLASHCARDS_LOCALE PROPERTY STRINGS en pl)

if(AI_FLASHCARDS_LOCALE STREQUAL "pl")
    set(AI_FLASHCARDS_LOCALE_DEFINE AI_FLASHCARDS_LOCALE_PL)
elseif(AI_FLASHCARDS_LOCALE STREQUAL "en")
    set(AI_FLASHCARDS_LOCALE_DEFINE AI_FLASHCARDS_LOCALE_EN)
else()
    message(FATAL_ERROR "Unsupported AI_FLASHCARDS_LOCALE='${AI_FLASHCARDS_LOCALE}'. Use 'pl' or 'en'.")
endif()

# ==========================================
# Build Speed Improvements (ccache)
# ==========================================
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Enabled ccache for faster recompilation")
endif()

# ==========================================
# Compiler Warnings (Best Practices)
# ==========================================
add_library(project_warnings INTERFACE)
if(MSVC)
    target_compile_options(project_warnings INTERFACE
        /W4
        /permissive-
        /utf-8
    )
else()
    target_compile_options(project_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )
endif()

# ==========================================
# FetchContent for dependencies
# ==========================================
include(FetchContent)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(nlohmann_json)

# FTXUI
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# ==========================================
# Core Library Target
# (Where our business logic will live)
# ==========================================
add_library(core_lib STATIC
    src/core_dummy.cpp
    src/FlashcardList.cpp
    src/DeckManager.cpp
    src/StudySession.cpp
    src/JsonStorage.cpp
    src/app/AppController.cpp
    src/app/presenters/ListsBrowserPresenter.cpp
    src/app/presenters/DeckEditorPresenter.cpp
    src/app/presenters/StudyConfigPresenter.cpp
    src/app/presenters/StudySessionPresenter.cpp
    src/app/views/FtxuiListsBrowserView.cpp
    src/app/views/FtxuiStudySessionView.cpp
    src/app/views/FtxuiDeckEditorView.cpp
    src/app/views/FtxuiStudyConfigView.cpp
)

# We will add source files here later in Milestone 1
# target_sources(core_lib PRIVATE src/Flashcard.cpp)

target_include_directories(core_lib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(core_lib PUBLIC ${AI_FLASHCARDS_LOCALE_DEFINE})
target_link_libraries(core_lib PUBLIC nlohmann_json::nlohmann_json ftxui::screen ftxui::dom ftxui::component PRIVATE project_warnings)

# ==========================================
# Executable Target (The actual app)
# ==========================================
add_executable(flashcards_app src/main.cpp)
target_link_libraries(flashcards_app PRIVATE core_lib ftxui::screen ftxui::dom ftxui::component project_warnings)

# ==========================================
# Testing Target
# ==========================================
enable_testing()
add_executable(flashcards_tests 
    tests/test_main.cpp
    tests/FlashcardList_test.cpp
    tests/DeckManager_test.cpp
    tests/StudySession_test.cpp
    tests/JsonStorage_test.cpp
    tests/JsonMappers_test.cpp
    tests/UndoManager_test.cpp
    tests/app/AppController_test.cpp
    tests/app/model/ListsBrowserViewModel_test.cpp
    tests/app/model/StudyConfigViewModel_test.cpp
    tests/app/presenters/ListsBrowserPresenter_test.cpp
    tests/app/presenters/DeckEditorPresenter_test.cpp
    tests/app/presenters/StudyConfigPresenter_test.cpp
    tests/app/presenters/StudySessionPresenter_test.cpp
)

# Link the test executable with GTest and our Core library
target_link_libraries(flashcards_tests PRIVATE 
    GTest::gtest_main 
    GTest::gmock
    core_lib 
    project_warnings
)

# Ensure test utilities can be included from anywhere in the tests directory
target_include_directories(flashcards_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/app
)

# Register the test executable with CTest
include(GoogleTest)
gtest_discover_tests(flashcards_tests)

# ==========================================
# Windows UTF-8 Manifest
# ==========================================
if(WIN32)
    set(MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/utf8_process.manifest")
    file(WRITE "${MANIFEST_FILE}"
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
"<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n"
"  <application>\n"
"    <windowsSettings>\n"
"      <activeCodePage xmlns=\"http://schemas.microsoft.com/SMI/2019/WindowsSettings\">UTF-8</activeCodePage>\n"
"    </windowsSettings>\n"
"  </application>\n"
"</assembly>\n"
    )
    target_sources(flashcards_app PRIVATE "${MANIFEST_FILE}")
    target_sources(flashcards_tests PRIVATE "${MANIFEST_FILE}")
endif()

# Install targets to bin directory
install(TARGETS flashcards_app DESTINATION bin)
install(TARGETS flashcards_tests DESTINATION bin)
