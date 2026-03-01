#include "data/world_persistence.h"
#include <fstream>
#include <vector>
#include <zlib.h>
#include "utils/logger.h"

namespace atlas {
namespace data {

bool WorldPersistence::saveWorldCompressed(const ecs::World* world,
                                           const std::string& filepath) {
    std::string json = serializeWorld(world);
    if (json.empty()) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Nothing to compress");
        return false;
    }

    gzFile gz = gzopen(filepath.c_str(), "wb9");  // max compression
    if (!gz) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Cannot open compressed file for writing: " + filepath);
        return false;
    }

    int written = gzwrite(gz, json.data(), static_cast<unsigned>(json.size()));
    gzclose(gz);

    if (written <= 0) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Compressed write failed");
        return false;
    }

    atlas::utils::Logger::instance().info("[WorldPersistence] World saved (compressed) to " + filepath + " (" + std::to_string(written) + " bytes source, compressed on disk)");
    return true;
}

bool WorldPersistence::loadWorldCompressed(ecs::World* world,
                                           const std::string& filepath) {
    gzFile gz = gzopen(filepath.c_str(), "rb");
    if (!gz) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Cannot open compressed file for reading: " + filepath);
        return false;
    }

    std::string json;
    char buf[8192];
    int n;
    while ((n = gzread(gz, buf, sizeof(buf))) > 0) {
        json.append(buf, static_cast<size_t>(n));
    }
    gzclose(gz);

    if (json.empty()) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Decompressed data is empty");
        return false;
    }

    return deserializeWorld(world, json);
}

} // namespace data
} // namespace atlas
