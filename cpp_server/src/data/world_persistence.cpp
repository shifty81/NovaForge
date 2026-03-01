#include "data/world_persistence.h"
#include <fstream>
#include <sstream>
#include "utils/logger.h"

namespace atlas {
namespace data {

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------

bool WorldPersistence::saveWorld(const ecs::World* world,
                                 const std::string& filepath) {
    std::string json = serializeWorld(world);

    std::ofstream file(filepath);
    if (!file.is_open()) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Cannot open file for writing: " + filepath);
        return false;
    }

    file << json;
    file.close();

    atlas::utils::Logger::instance().info("[WorldPersistence] World saved to " + filepath);
    return true;
}

bool WorldPersistence::loadWorld(ecs::World* world,
                                 const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Cannot open file for reading: " + filepath);
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    return deserializeWorld(world, ss.str());
}

} // namespace data
} // namespace atlas
