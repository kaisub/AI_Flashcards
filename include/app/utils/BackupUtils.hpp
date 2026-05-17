#pragma once

#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <string_view>
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

// Restores the local "data" directory from a .zip backup.
// Safety: archive is always unpacked to a temporary directory first and validated.
// Only after successful unpack+validation do we replace the existing "data" directory.
inline bool restoreDataFromBackup(const std::filesystem::path& archivePath, std::string* diagnostic = nullptr) {
    auto fail = [&](std::string_view message) {
        if (diagnostic) {
            *diagnostic = std::string(message);
        }
        return false;
    };

    std::error_code ec;
    if (!std::filesystem::exists(archivePath, ec) || !std::filesystem::is_regular_file(archivePath, ec)) {
        return fail("backup file does not exist");
    }

    const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
    const std::filesystem::path tempRoot =
        std::filesystem::current_path(ec) / (".flashcards_restore_" + std::to_string(stamp));
    if (ec) {
        return fail("cannot create temporary restore path");
    }

    const std::filesystem::path unpackDir = tempRoot / "unpacked";
    std::filesystem::create_directories(unpackDir, ec);
    if (ec) {
        return fail("cannot create temporary restore directory");
    }

    const std::string unzipCmd = "unzip -q " + backupShellQuote(archivePath.string()) +
                                 " -d " + backupShellQuote(unpackDir.string());
    const int unzipRc = std::system(unzipCmd.c_str());  // NOLINT(concurrency-mt-unsafe)
    if (unzipRc != 0) {
        std::filesystem::remove_all(tempRoot, ec);
        return fail("unzip failed");
    }

    const std::filesystem::path extractedData = unpackDir / "data";
    if (!std::filesystem::exists(extractedData, ec) || !std::filesystem::is_directory(extractedData, ec)) {
        std::filesystem::remove_all(tempRoot, ec);
        return fail("archive does not contain top-level data folder");
    }

    const std::filesystem::path targetData{"data"};
    const std::filesystem::path oldDataBackup = tempRoot / "old_data";

    const bool hadOldData = std::filesystem::exists(targetData, ec) && std::filesystem::is_directory(targetData, ec);
    if (ec) {
        std::filesystem::remove_all(tempRoot, ec);
        return fail("cannot inspect current data folder");
    }

    if (hadOldData) {
        std::filesystem::rename(targetData, oldDataBackup, ec);
        if (ec) {
            std::filesystem::remove_all(tempRoot, ec);
            return fail("cannot stage current data for rollback");
        }
    }

    std::filesystem::copy(
        extractedData,
        targetData,
        std::filesystem::copy_options::recursive,
        ec);
    if (ec) {
        std::error_code cleanupEc;
        std::filesystem::remove_all(targetData, cleanupEc);
        if (hadOldData) {
            std::error_code rollbackEc;
            std::filesystem::rename(oldDataBackup, targetData, rollbackEc);
        }
        std::filesystem::remove_all(tempRoot, ec);
        return fail("cannot copy restored data into workspace");
    }

    if (hadOldData) {
        std::filesystem::remove_all(oldDataBackup, ec);
    }
    std::filesystem::remove_all(tempRoot, ec);
    if (diagnostic) {
        diagnostic->clear();
    }
    return true;
}

} // namespace app::utils
