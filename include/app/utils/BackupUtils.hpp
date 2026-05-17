#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

namespace app::utils {

// Wraps a value in single-quotes safe for POSIX shell command arguments.
inline std::string backupShellQuote(const std::string& value) {
    std::string quoted = "'";
    for (char c : value) {
        if (c == '\'') {
            quoted += "'\\''";
        } else {
            quoted += c;
        }
    }
    quoted += "'";
    return quoted;
}

// Packs the "data" directory relative to CWD into a ZIP archive at outputPath.
// Appends ".zip" to outputPath if the extension is missing.
// Creates intermediate directories as needed.
// Removes any existing file at the resolved path before writing.
// Returns true on success.
inline bool createZipBackup(const std::filesystem::path& outputPath) {
    const std::filesystem::path sourceDir{"data"};
    if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir)) {
        return false;
    }

    std::filesystem::path normalized = outputPath;
    if (normalized.extension() != ".zip") {
        normalized += ".zip";
    }

    if (normalized.has_parent_path() && !normalized.parent_path().empty()) {
        std::error_code ec;
        std::filesystem::create_directories(normalized.parent_path(), ec);
        if (ec) {
            return false;
        }
    }

    std::error_code rmEc;
    std::filesystem::remove(normalized, rmEc);

    const std::string cmd = "zip -r -q " + backupShellQuote(normalized.string())
                          + " " + backupShellQuote(sourceDir.string());
    return std::system(cmd.c_str()) == 0;  // NOLINT(concurrency-mt-unsafe)
}

// Computes the full output path from a directory and filename, appending ".zip" if needed.
inline std::filesystem::path buildBackupOutputPath(const std::filesystem::path& dir, const std::string& filename) {
    std::filesystem::path p = dir / filename;
    if (p.extension() != ".zip") {
        p += ".zip";
    }
    return p;
}

} // namespace app::utils
