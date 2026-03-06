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

        // Check exe directory itself
        std::filesystem::path resolved = exeDirPath / filePath;
        if (std::filesystem::exists(resolved)) {
            return resolved.string();
        }

        // Walk up parent directories (handles multi-config generators like
        // Visual Studio where the exe may be in bin/Release/ or bin/Debug/
        // while resources are at the project root).
        std::filesystem::path parent = exeDirPath;
        for (int i = 0; i < 6; ++i) {
            parent = parent.parent_path();
            if (parent.empty() || parent == parent.parent_path()) break;
            resolved = parent / filePath;
            if (std::filesystem::exists(resolved)) {
                return resolved.string();
            }
            // Also check under cpp_client/ subdirectory (handles builds
            // from the root directory where shaders live in cpp_client/)
            resolved = parent / "cpp_client" / filePath;
            if (std::filesystem::exists(resolved)) {
                return resolved.string();
            }
        }
    }

    // Return original path (will produce the same "failed to open" error)
    return filePath;
}

} // namespace atlas
