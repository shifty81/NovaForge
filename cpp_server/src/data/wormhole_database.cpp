#include "data/wormhole_database.h"
#include "utils/json_helpers.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include "utils/logger.h"

namespace atlas {
namespace data {

using json::extractString;
using json::extractFloat;
using json::extractInt;

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

int WormholeDatabase::loadFromDirectory(const std::string& data_dir) {
    int total = 0;
    total += loadClasses(data_dir + "/wormholes/wormhole_classes.json");
    total += loadEffects(data_dir + "/wormholes/wormhole_effects.json");
    atlas::utils::Logger::instance().info("[WormholeDatabase] Loaded " + std::to_string(classes_.size()) + " wormhole classes and " + std::to_string(effects_.size()) + " effects from " + data_dir);
    return total;
}

const WormholeClassTemplate* WormholeDatabase::getWormholeClass(const std::string& class_id) const {
    auto it = classes_.find(class_id);
    return (it != classes_.end()) ? &it->second : nullptr;
}

const WormholeEffect* WormholeDatabase::getEffect(const std::string& effect_id) const {
    auto it = effects_.find(effect_id);
    return (it != effects_.end()) ? &it->second : nullptr;
}

std::vector<std::string> WormholeDatabase::getClassIds() const {
    std::vector<std::string> ids;
    ids.reserve(classes_.size());
    for (const auto& kv : classes_) ids.push_back(kv.first);
    return ids;
}

std::vector<std::string> WormholeDatabase::getEffectIds() const {
    std::vector<std::string> ids;
    ids.reserve(effects_.size());
    for (const auto& kv : effects_) ids.push_back(kv.first);
    return ids;
}

// ---------------------------------------------------------------------------
// Loaders
// ---------------------------------------------------------------------------

int WormholeDatabase::loadClasses(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) return 0;

    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string content = buf.str();

    int loaded = 0;
    size_t pos = 0;

    while (pos < content.size()) {
        size_t key_start = content.find('\"', pos);
        if (key_start == std::string::npos) break;
        size_t key_end = content.find('\"', key_start + 1);
        if (key_end == std::string::npos) break;

        std::string key = content.substr(key_start + 1, key_end - key_start - 1);

        size_t block_start = content.find('{', key_end);
        if (block_start == std::string::npos) break;

        size_t block_end = json::findBlockEnd(content, block_start, '{', '}');
        if (block_end == std::string::npos) break;

        std::string block = content.substr(block_start, block_end - block_start + 1);
        if (key.empty() || block.size() <= 2) { pos = block_end + 1; continue; }

        WormholeClassTemplate tmpl;
        tmpl.id   = key;
        tmpl.name = extractString(block, "name");
        if (tmpl.name.empty()) { pos = block_end + 1; continue; }

        tmpl.wormhole_class          = extractInt(block, "class", 1);
        tmpl.difficulty              = extractString(block, "difficulty");
        tmpl.description             = extractString(block, "description");
        tmpl.max_ship_class          = extractString(block, "max_ship_class");
        tmpl.max_ship_mass           = static_cast<double>(extractFloat(block, "max_ship_mass", 20000000.0f));
        tmpl.max_wormhole_stability  = static_cast<double>(extractFloat(block, "max_wormhole_stability", 500000000.0f));
        tmpl.max_wormhole_lifetime_hours = extractFloat(block, "max_wormhole_lifetime_hours", 24.0f);
        tmpl.salvage_value_multiplier = extractFloat(block, "salvage_value_multiplier", 1.0f);
        tmpl.blue_loot_isc           = static_cast<double>(extractFloat(block, "blue_loot_isc", 150000.0f));

        // Static connections
        std::string sc_arr = json::extractArray(block, "static_connections");
        if (!sc_arr.empty()) tmpl.static_connections = json::parseStringArray(sc_arr);

        // Dormant NPC spawns
        std::string spawns_arr = json::extractArray(block, "dormant_spawns");
        if (!spawns_arr.empty()) {
            // Each element is { ... }
            size_t sp = 0;
            while (sp < spawns_arr.size()) {
                size_t obj_start = spawns_arr.find('{', sp);
                if (obj_start == std::string::npos) break;
                size_t obj_end = json::findBlockEnd(spawns_arr, obj_start, '{', '}');
                if (obj_end == std::string::npos) break;
                std::string obj = spawns_arr.substr(obj_start, obj_end - obj_start + 1);

                DormantSpawn spawn;
                spawn.id        = extractString(obj, "id");
                spawn.name      = extractString(obj, "name");
                spawn.type      = extractString(obj, "type");
                spawn.count_min = extractInt(obj, "count_min", 1);
                spawn.count_max = extractInt(obj, "count_max", 2);
                if (!spawn.id.empty()) tmpl.dormant_spawns.push_back(std::move(spawn));

                sp = obj_end + 1;
            }
        }

        classes_[key] = std::move(tmpl);
        ++loaded;
        pos = block_end + 1;
    }
    return loaded;
}

int WormholeDatabase::loadEffects(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) return 0;

    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string content = buf.str();

    int loaded = 0;
    size_t pos = 0;

    while (pos < content.size()) {
        size_t key_start = content.find('\"', pos);
        if (key_start == std::string::npos) break;
        size_t key_end = content.find('\"', key_start + 1);
        if (key_end == std::string::npos) break;

        std::string key = content.substr(key_start + 1, key_end - key_start - 1);

        size_t block_start = content.find('{', key_end);
        if (block_start == std::string::npos) break;

        size_t block_end = json::findBlockEnd(content, block_start, '{', '}');
        if (block_end == std::string::npos) break;

        std::string block = content.substr(block_start, block_end - block_start + 1);
        if (key.empty() || block.size() <= 2) { pos = block_end + 1; continue; }

        WormholeEffect eff;
        eff.id          = key;
        eff.name        = extractString(block, "name");
        eff.description = extractString(block, "description");
        if (eff.name.empty()) { pos = block_end + 1; continue; }

        // Parse modifiers block
        std::string mod_block = json::extractObject(block, "modifiers");
        if (!mod_block.empty()) {
            // Walk through key-value pairs inside the modifiers object
            size_t mp = 1; // skip opening {
            while (mp < mod_block.size()) {
                size_t mk_start = mod_block.find('\"', mp);
                if (mk_start == std::string::npos) break;
                size_t mk_end = mod_block.find('\"', mk_start + 1);
                if (mk_end == std::string::npos) break;
                std::string mk = mod_block.substr(mk_start + 1, mk_end - mk_start - 1);

                size_t colon = mod_block.find(':', mk_end);
                if (colon == std::string::npos) break;

                float val = 1.0f;
                size_t vp = colon + 1;
                while (vp < mod_block.size() && (mod_block[vp] == ' ' || mod_block[vp] == '\t'))
                    ++vp;
                size_t ve = vp;
                while (ve < mod_block.size() && (mod_block[ve] == '-' || mod_block[ve] == '.' ||
                       (mod_block[ve] >= '0' && mod_block[ve] <= '9')))
                    ++ve;
                if (ve > vp) {
                    try { val = std::stof(mod_block.substr(vp, ve - vp)); } catch (const std::exception& e) {
                        atlas::utils::Logger::instance().warn(std::string("[WormholeDatabase] Failed to parse modifier value: ") + e.what());
                    }
                }

                if (!mk.empty()) eff.modifiers[mk] = val;
                mp = ve;
            }
        }

        effects_[key] = std::move(eff);
        ++loaded;
        pos = block_end + 1;
    }
    return loaded;
}

} // namespace data
} // namespace atlas
