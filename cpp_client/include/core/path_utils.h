#pragma once
#include <string>
#include <filesystem>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <climits>
#else
#include <unistd.h>
#include <climits>
#endif

namespace atlas {

/// Returns the directory containing the running executable.
inline std::string getExecutableDir() {
    std::filesystem::path exePath;
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        exePath = buf;
    }
#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        exePath = std::filesystem::canonical(buf);
    }
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        exePath = buf;
    }
#endif
    if (!exePath.empty()) {
        return exePath.parent_path().string();
    }
    return "";
}

/// Resolve a file path by first checking as-is, then relative to the executable.
inline std::string resolvePath(const std::string& filePath) {
    // First, try the path directly (works when CWD is correct)
    if (std::filesystem::exists(filePath)) {
        return filePath;
    }

    // Fall back to resolving relative to the executable directory
    std::string exeDir = getExecutableDir();
    if (!exeDir.empty()) {
        std::filesystem::path exeDirPath(exeDir);
        std::filesystem::path resolved = exeDirPath / filePath;
        if (std::filesystem::exists(resolved)) {
            return resolved.string();
        }

        // For multi-config generators (e.g., Visual Studio) the executable
        // may be in a configuration subdirectory such as bin/Release/ or
        // bin/Debug/ while resources are placed in the parent bin/ directory.
        // Check the parent directory as well.
        resolved = exeDirPath.parent_path() / filePath;
        if (std::filesystem::exists(resolved)) {
            return resolved.string();
        }
    }

    // Return original path (will produce the same "failed to open" error)
    return filePath;
}

} // namespace atlas
