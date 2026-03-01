#include "data/world_persistence.h"
#include "utils/json_helpers.h"
#include <string>

namespace atlas {
namespace data {

std::string WorldPersistence::extractString(const std::string& json,
                                            const std::string& key) {
    return atlas::json::extractString(json, key);
}

float WorldPersistence::extractFloat(const std::string& json,
                                     const std::string& key,
                                     float fallback) {
    return atlas::json::extractFloat(json, key, fallback);
}

int WorldPersistence::extractInt(const std::string& json,
                                 const std::string& key,
                                 int fallback) {
    return atlas::json::extractInt(json, key, fallback);
}

double WorldPersistence::extractDouble(const std::string& json,
                                       const std::string& key,
                                       double fallback) {
    return atlas::json::extractDouble(json, key, fallback);
}

bool WorldPersistence::extractBool(const std::string& json,
                                   const std::string& key,
                                   bool fallback) {
    return atlas::json::extractBool(json, key, fallback);
}

std::string WorldPersistence::extractObject(const std::string& json,
                                            const std::string& key) {
    return atlas::json::extractObject(json, key);
}

} // namespace data
} // namespace atlas
