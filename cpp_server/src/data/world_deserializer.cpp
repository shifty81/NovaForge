#include "data/world_persistence.h"
#include "components/game_components.h"
#include "utils/logger.h"

namespace atlas {
namespace data {

bool WorldPersistence::deserializeWorld(ecs::World* world,
                                        const std::string& json) const {
    // Find the entities array
    size_t arr_start = json.find("[");
    size_t arr_end   = json.rfind("]");
    if (arr_start == std::string::npos || arr_end == std::string::npos ||
        arr_end <= arr_start) {
        atlas::utils::Logger::instance().error("[WorldPersistence] Invalid JSON structure");
        return false;
    }

    std::string array_content = json.substr(arr_start + 1,
                                            arr_end - arr_start - 1);

    // Parse individual entity objects by matching braces
    int depth = 0;
    size_t obj_start = std::string::npos;
    int entity_count = 0;

    for (size_t i = 0; i < array_content.size(); ++i) {
        char c = array_content[i];
        if (c == '{') {
            if (depth == 0) obj_start = i;
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0 && obj_start != std::string::npos) {
                std::string entity_json =
                    array_content.substr(obj_start, i - obj_start + 1);
                if (deserializeEntity(world, entity_json)) {
                    ++entity_count;
                }
                obj_start = std::string::npos;
            }
        }
    }

    atlas::utils::Logger::instance().info("[WorldPersistence] Loaded " + std::to_string(entity_count) + " entities");
    return true;
}

bool WorldPersistence::deserializeEntity(ecs::World* world,
                                          const std::string& json) const {
    std::string id = extractString(json, "id");
    if (id.empty()) return false;

    auto* entity = world->createEntity(id);
    if (!entity) return false;

    // Position
    std::string pos_json = extractObject(json, "position");
    if (!pos_json.empty()) {
        auto pos = std::make_unique<components::Position>();
        pos->x = extractFloat(pos_json, "x");
        pos->y = extractFloat(pos_json, "y");
        pos->z = extractFloat(pos_json, "z");
        pos->rotation = extractFloat(pos_json, "rotation");
        entity->addComponent(std::move(pos));
    }

    // Velocity
    std::string vel_json = extractObject(json, "velocity");
    if (!vel_json.empty()) {
        auto vel = std::make_unique<components::Velocity>();
        vel->vx = extractFloat(vel_json, "vx");
        vel->vy = extractFloat(vel_json, "vy");
        vel->vz = extractFloat(vel_json, "vz");
        vel->angular_velocity = extractFloat(vel_json, "angular_velocity");
        vel->max_speed = extractFloat(vel_json, "max_speed", 100.0f);
        entity->addComponent(std::move(vel));
    }

    // Health
    std::string hp_json = extractObject(json, "health");
    if (!hp_json.empty()) {
        auto hp = std::make_unique<components::Health>();
        hp->hull_hp   = extractFloat(hp_json, "hull_hp", 100.0f);
        hp->hull_max  = extractFloat(hp_json, "hull_max", 100.0f);
        hp->armor_hp  = extractFloat(hp_json, "armor_hp", 100.0f);
        hp->armor_max = extractFloat(hp_json, "armor_max", 100.0f);
        hp->shield_hp  = extractFloat(hp_json, "shield_hp", 100.0f);
        hp->shield_max = extractFloat(hp_json, "shield_max", 100.0f);
        hp->shield_recharge_rate = extractFloat(hp_json, "shield_recharge_rate", 1.0f);
        hp->hull_em_resist        = extractFloat(hp_json, "hull_em_resist");
        hp->hull_thermal_resist   = extractFloat(hp_json, "hull_thermal_resist");
        hp->hull_kinetic_resist   = extractFloat(hp_json, "hull_kinetic_resist");
        hp->hull_explosive_resist = extractFloat(hp_json, "hull_explosive_resist");
        hp->armor_em_resist        = extractFloat(hp_json, "armor_em_resist");
        hp->armor_thermal_resist   = extractFloat(hp_json, "armor_thermal_resist");
        hp->armor_kinetic_resist   = extractFloat(hp_json, "armor_kinetic_resist");
        hp->armor_explosive_resist = extractFloat(hp_json, "armor_explosive_resist");
        hp->shield_em_resist        = extractFloat(hp_json, "shield_em_resist");
        hp->shield_thermal_resist   = extractFloat(hp_json, "shield_thermal_resist");
        hp->shield_kinetic_resist   = extractFloat(hp_json, "shield_kinetic_resist");
        hp->shield_explosive_resist = extractFloat(hp_json, "shield_explosive_resist");
        entity->addComponent(std::move(hp));
    }

    // Capacitor
    std::string cap_json = extractObject(json, "capacitor");
    if (!cap_json.empty()) {
        auto cap = std::make_unique<components::Capacitor>();
        cap->capacitor     = extractFloat(cap_json, "capacitor", 100.0f);
        cap->capacitor_max = extractFloat(cap_json, "capacitor_max", 100.0f);
        cap->recharge_rate = extractFloat(cap_json, "recharge_rate", 2.0f);
        entity->addComponent(std::move(cap));
    }

    // Ship
    std::string ship_json = extractObject(json, "ship");
    if (!ship_json.empty()) {
        auto ship = std::make_unique<components::Ship>();
        ship->ship_type  = extractString(ship_json, "ship_type");
        ship->ship_class = extractString(ship_json, "ship_class");
        ship->ship_name  = extractString(ship_json, "ship_name");
        ship->race       = extractString(ship_json, "race");
        ship->cpu        = extractFloat(ship_json, "cpu");
        ship->cpu_max    = extractFloat(ship_json, "cpu_max", 100.0f);
        ship->powergrid     = extractFloat(ship_json, "powergrid");
        ship->powergrid_max = extractFloat(ship_json, "powergrid_max", 50.0f);
        ship->signature_radius    = extractFloat(ship_json, "signature_radius", 35.0f);
        ship->scan_resolution     = extractFloat(ship_json, "scan_resolution", 400.0f);
        ship->max_locked_targets  = extractInt(ship_json, "max_locked_targets", 3);
        ship->max_targeting_range = extractFloat(ship_json, "max_targeting_range", 20000.0f);
        entity->addComponent(std::move(ship));
    }

    // Faction
    std::string fac_json = extractObject(json, "faction");
    if (!fac_json.empty()) {
        auto fac = std::make_unique<components::Faction>();
        fac->faction_name = extractString(fac_json, "faction_name");
        entity->addComponent(std::move(fac));
    }

    // Standings
    std::string standings_json = extractObject(json, "standings");
    if (!standings_json.empty()) {
        auto standings = std::make_unique<components::Standings>();
        
        // Deserialize personal standings
        std::string personal_json = extractObject(standings_json, "personal");
        if (!personal_json.empty()) {
            // Parse the personal standings map
            // Format: {"entity_id": standing_value, ...}
            size_t pos = 0;
            while (pos < personal_json.size()) {
                // Find next key
                size_t key_start = personal_json.find("\"", pos);
                if (key_start == std::string::npos) break;
                key_start++;
                size_t key_end = personal_json.find("\"", key_start);
                if (key_end == std::string::npos) break;
                
                std::string entity_id = personal_json.substr(key_start, key_end - key_start);
                
                // Find value after colon
                size_t colon = personal_json.find(":", key_end);
                if (colon == std::string::npos) break;
                
                // Extract number
                size_t val_start = colon + 1;
                while (val_start < personal_json.size() && 
                       (personal_json[val_start] == ' ' || personal_json[val_start] == '\t')) {
                    val_start++;
                }
                size_t val_end = val_start;
                while (val_end < personal_json.size() && 
                       (std::isdigit(personal_json[val_end]) || personal_json[val_end] == '.' || 
                        personal_json[val_end] == '-' || personal_json[val_end] == '+')) {
                    val_end++;
                }
                
                if (val_end > val_start) {
                    float standing = std::stof(personal_json.substr(val_start, val_end - val_start));
                    standings->personal_standings[entity_id] = standing;
                }
                
                pos = val_end + 1;
            }
        }
        
        // Deserialize corporation standings
        std::string corp_json = extractObject(standings_json, "corporation");
        if (!corp_json.empty()) {
            size_t pos = 0;
            while (pos < corp_json.size()) {
                size_t key_start = corp_json.find("\"", pos);
                if (key_start == std::string::npos) break;
                key_start++;
                size_t key_end = corp_json.find("\"", key_start);
                if (key_end == std::string::npos) break;
                
                std::string corp_name = corp_json.substr(key_start, key_end - key_start);
                size_t colon = corp_json.find(":", key_end);
                if (colon == std::string::npos) break;
                
                size_t val_start = colon + 1;
                while (val_start < corp_json.size() && 
                       (corp_json[val_start] == ' ' || corp_json[val_start] == '\t')) {
                    val_start++;
                }
                size_t val_end = val_start;
                while (val_end < corp_json.size() && 
                       (std::isdigit(corp_json[val_end]) || corp_json[val_end] == '.' || 
                        corp_json[val_end] == '-' || corp_json[val_end] == '+')) {
                    val_end++;
                }
                
                if (val_end > val_start) {
                    float standing = std::stof(corp_json.substr(val_start, val_end - val_start));
                    standings->corporation_standings[corp_name] = standing;
                }
                
                pos = val_end + 1;
            }
        }
        
        // Deserialize faction standings
        std::string faction_json = extractObject(standings_json, "faction");
        if (!faction_json.empty()) {
            size_t pos = 0;
            while (pos < faction_json.size()) {
                size_t key_start = faction_json.find("\"", pos);
                if (key_start == std::string::npos) break;
                key_start++;
                size_t key_end = faction_json.find("\"", key_start);
                if (key_end == std::string::npos) break;
                
                std::string faction_name = faction_json.substr(key_start, key_end - key_start);
                size_t colon = faction_json.find(":", key_end);
                if (colon == std::string::npos) break;
                
                size_t val_start = colon + 1;
                while (val_start < faction_json.size() && 
                       (faction_json[val_start] == ' ' || faction_json[val_start] == '\t')) {
                    val_start++;
                }
                size_t val_end = val_start;
                while (val_end < faction_json.size() && 
                       (std::isdigit(faction_json[val_end]) || faction_json[val_end] == '.' || 
                        faction_json[val_end] == '-' || faction_json[val_end] == '+')) {
                    val_end++;
                }
                
                if (val_end > val_start) {
                    float standing = std::stof(faction_json.substr(val_start, val_end - val_start));
                    standings->faction_standings[faction_name] = standing;
                }
                
                pos = val_end + 1;
            }
        }
        
        entity->addComponent(std::move(standings));
    }

    // AI
    std::string ai_json = extractObject(json, "ai");
    if (!ai_json.empty()) {
        auto ai = std::make_unique<components::AI>();
        ai->behavior = static_cast<components::AI::Behavior>(
            extractInt(ai_json, "behavior"));
        ai->state = static_cast<components::AI::State>(
            extractInt(ai_json, "state"));
        ai->target_entity_id = extractString(ai_json, "target_entity_id");
        ai->orbit_distance   = extractFloat(ai_json, "orbit_distance", 1000.0f);
        ai->awareness_range  = extractFloat(ai_json, "awareness_range", 50000.0f);
        entity->addComponent(std::move(ai));
    }

    // Weapon
    std::string wep_json = extractObject(json, "weapon");
    if (!wep_json.empty()) {
        auto weapon = std::make_unique<components::Weapon>();
        weapon->weapon_type    = extractString(wep_json, "weapon_type");
        weapon->damage_type    = extractString(wep_json, "damage_type");
        weapon->damage         = extractFloat(wep_json, "damage", 10.0f);
        weapon->optimal_range  = extractFloat(wep_json, "optimal_range", 5000.0f);
        weapon->falloff_range  = extractFloat(wep_json, "falloff_range", 2500.0f);
        weapon->tracking_speed = extractFloat(wep_json, "tracking_speed", 0.5f);
        weapon->rate_of_fire   = extractFloat(wep_json, "rate_of_fire", 3.0f);
        weapon->capacitor_cost = extractFloat(wep_json, "capacitor_cost", 5.0f);
        weapon->ammo_type      = extractString(wep_json, "ammo_type");
        weapon->ammo_count     = extractInt(wep_json, "ammo_count", 100);
        entity->addComponent(std::move(weapon));
    }

    // Player
    std::string player_json = extractObject(json, "player");
    if (!player_json.empty()) {
        auto player = std::make_unique<components::Player>();
        player->player_id      = extractString(player_json, "player_id");
        player->character_name = extractString(player_json, "character_name");
        player->credits            = extractDouble(player_json, "credits", 1000000.0);
        player->corporation    = extractString(player_json, "corporation");
        entity->addComponent(std::move(player));
    }

    // WormholeConnection
    std::string wh_json = extractObject(json, "wormhole_connection");
    if (!wh_json.empty()) {
        auto wh = std::make_unique<components::WormholeConnection>();
        wh->wormhole_id         = extractString(wh_json, "wormhole_id");
        wh->source_system       = extractString(wh_json, "source_system");
        wh->destination_system  = extractString(wh_json, "destination_system");
        wh->max_mass            = extractDouble(wh_json, "max_mass", 500000000.0);
        wh->remaining_mass      = extractDouble(wh_json, "remaining_mass", 500000000.0);
        wh->max_jump_mass       = extractDouble(wh_json, "max_jump_mass", 20000000.0);
        wh->max_lifetime_hours  = extractFloat(wh_json, "max_lifetime_hours", 24.0f);
        wh->elapsed_hours       = extractFloat(wh_json, "elapsed_hours");
        wh->collapsed           = extractBool(wh_json, "collapsed");
        entity->addComponent(std::move(wh));
    }

    // SolarSystem
    std::string ss_json = extractObject(json, "solar_system");
    if (!ss_json.empty()) {
        auto ss = std::make_unique<components::SolarSystem>();
        ss->system_id       = extractString(ss_json, "system_id");
        ss->system_name     = extractString(ss_json, "system_name");
        ss->wormhole_class  = extractInt(ss_json, "wormhole_class");
        ss->effect_name     = extractString(ss_json, "effect_name");
        ss->dormants_spawned = extractBool(ss_json, "dormants_spawned");
        entity->addComponent(std::move(ss));
    }

    // FleetMembership
    std::string fm_json = extractObject(json, "fleet_membership");
    if (!fm_json.empty()) {
        auto fm = std::make_unique<components::FleetMembership>();
        fm->fleet_id = extractString(fm_json, "fleet_id");
        fm->role     = extractString(fm_json, "role");
        fm->squad_id = extractString(fm_json, "squad_id");
        fm->wing_id  = extractString(fm_json, "wing_id");
        entity->addComponent(std::move(fm));
    }

    // Inventory
    std::string inv_json = extractObject(json, "inventory");
    if (!inv_json.empty()) {
        auto inv = std::make_unique<components::Inventory>();
        inv->max_capacity = extractFloat(inv_json, "max_capacity", 400.0f);

        // Parse items array
        size_t arr_start = inv_json.find("[");
        size_t arr_end = inv_json.rfind("]");
        if (arr_start != std::string::npos && arr_end != std::string::npos && arr_end > arr_start) {
            std::string items_content = inv_json.substr(arr_start + 1, arr_end - arr_start - 1);
            int depth = 0;
            size_t obj_start = std::string::npos;
            for (size_t i = 0; i < items_content.size(); ++i) {
                if (items_content[i] == '{') {
                    if (depth == 0) obj_start = i;
                    ++depth;
                } else if (items_content[i] == '}') {
                    --depth;
                    if (depth == 0 && obj_start != std::string::npos) {
                        std::string item_json = items_content.substr(obj_start, i - obj_start + 1);
                        components::Inventory::Item item;
                        item.item_id  = extractString(item_json, "item_id");
                        item.name     = extractString(item_json, "name");
                        item.type     = extractString(item_json, "type");
                        item.quantity = extractInt(item_json, "quantity");
                        item.volume   = extractFloat(item_json, "volume", 1.0f);
                        inv->items.push_back(item);
                        obj_start = std::string::npos;
                    }
                }
            }
        }
        entity->addComponent(std::move(inv));
    }

    // LootTable
    std::string lt_json = extractObject(json, "loot_table");
    if (!lt_json.empty()) {
        auto lt = std::make_unique<components::LootTable>();
        lt->isk_drop = extractDouble(lt_json, "isk_drop");

        // Parse entries array
        size_t arr_start = lt_json.find("[");
        size_t arr_end = lt_json.rfind("]");
        if (arr_start != std::string::npos && arr_end != std::string::npos && arr_end > arr_start) {
            std::string entries_content = lt_json.substr(arr_start + 1, arr_end - arr_start - 1);
            int depth = 0;
            size_t obj_start = std::string::npos;
            for (size_t i = 0; i < entries_content.size(); ++i) {
                if (entries_content[i] == '{') {
                    if (depth == 0) obj_start = i;
                    ++depth;
                } else if (entries_content[i] == '}') {
                    --depth;
                    if (depth == 0 && obj_start != std::string::npos) {
                        std::string entry_json = entries_content.substr(obj_start, i - obj_start + 1);
                        components::LootTable::LootEntry entry;
                        entry.item_id      = extractString(entry_json, "item_id");
                        entry.name         = extractString(entry_json, "name");
                        entry.type         = extractString(entry_json, "type");
                        entry.drop_chance  = extractFloat(entry_json, "drop_chance", 1.0f);
                        entry.min_quantity = extractInt(entry_json, "min_quantity", 1);
                        entry.max_quantity = extractInt(entry_json, "max_quantity", 1);
                        entry.volume       = extractFloat(entry_json, "volume", 1.0f);
                        lt->entries.push_back(entry);
                        obj_start = std::string::npos;
                    }
                }
            }
        }
        entity->addComponent(std::move(lt));
    }

    // Corporation
    std::string corp_json = extractObject(json, "corporation_data");
    if (!corp_json.empty()) {
        auto corp = std::make_unique<components::Corporation>();
        corp->corp_id    = extractString(corp_json, "corp_id");
        corp->corp_name  = extractString(corp_json, "corp_name");
        corp->ticker     = extractString(corp_json, "ticker");
        corp->ceo_id     = extractString(corp_json, "ceo_id");
        corp->tax_rate   = extractFloat(corp_json, "tax_rate", 0.05f);
        corp->corp_wallet = extractDouble(corp_json, "corp_wallet", 0.0);

        // Parse member_ids string array
        std::string mid_key = "\"member_ids\"";
        size_t mid_pos = corp_json.find(mid_key);
        if (mid_pos != std::string::npos) {
            size_t arr_s = corp_json.find("[", mid_pos);
            size_t arr_e = corp_json.find("]", arr_s);
            if (arr_s != std::string::npos && arr_e != std::string::npos) {
                std::string arr_content = corp_json.substr(arr_s + 1, arr_e - arr_s - 1);
                size_t q1 = 0;
                while ((q1 = arr_content.find("\"", q1)) != std::string::npos) {
                    size_t q2 = arr_content.find("\"", q1 + 1);
                    if (q2 == std::string::npos) break;
                    corp->member_ids.push_back(arr_content.substr(q1 + 1, q2 - q1 - 1));
                    q1 = q2 + 1;
                }
            }
        }

        // Parse hangar_items array
        std::string hi_key = "\"hangar_items\"";
        size_t hi_pos = corp_json.find(hi_key);
        if (hi_pos != std::string::npos) {
            size_t arr_start = corp_json.find("[", hi_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < corp_json.size() && bracket_depth > 0) {
                    if (corp_json[arr_end] == '[') ++bracket_depth;
                    else if (corp_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = corp_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string ij = content.substr(obj_start, i - obj_start + 1);
                                components::Corporation::CorpHangarItem item;
                                item.item_id  = extractString(ij, "item_id");
                                item.name     = extractString(ij, "name");
                                item.type     = extractString(ij, "type");
                                item.quantity = extractInt(ij, "quantity");
                                item.volume   = extractFloat(ij, "volume", 1.0f);
                                corp->hangar_items.push_back(item);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }

        entity->addComponent(std::move(corp));
    }

    // DroneBay
    std::string db_json = extractObject(json, "drone_bay");
    if (!db_json.empty()) {
        auto db = std::make_unique<components::DroneBay>();
        db->bay_capacity  = extractFloat(db_json, "bay_capacity", 25.0f);
        db->max_bandwidth = extractInt(db_json, "max_bandwidth", 25);

        // Helper lambda to parse a drone array
        auto parseDrones = [&](const std::string& array_key,
                               std::vector<components::DroneBay::DroneInfo>& out) {
            std::string key_search = "\"" + array_key + "\"";
            size_t key_pos = db_json.find(key_search);
            if (key_pos == std::string::npos) return;
            size_t arr_start = db_json.find("[", key_pos);
            // Find matching close bracket
            if (arr_start == std::string::npos) return;
            int bracket_depth = 1;
            size_t arr_end = arr_start + 1;
            while (arr_end < db_json.size() && bracket_depth > 0) {
                if (db_json[arr_end] == '[') ++bracket_depth;
                else if (db_json[arr_end] == ']') --bracket_depth;
                if (bracket_depth > 0) ++arr_end;
            }
            if (bracket_depth != 0) return;

            std::string content = db_json.substr(arr_start + 1, arr_end - arr_start - 1);
            int depth = 0;
            size_t obj_start = std::string::npos;
            for (size_t i = 0; i < content.size(); ++i) {
                if (content[i] == '{') {
                    if (depth == 0) obj_start = i;
                    ++depth;
                } else if (content[i] == '}') {
                    --depth;
                    if (depth == 0 && obj_start != std::string::npos) {
                        std::string dj = content.substr(obj_start, i - obj_start + 1);
                        components::DroneBay::DroneInfo info;
                        info.drone_id      = extractString(dj, "drone_id");
                        info.name          = extractString(dj, "name");
                        info.type          = extractString(dj, "type");
                        info.damage_type   = extractString(dj, "damage_type");
                        info.damage        = extractFloat(dj, "damage", 0.0f);
                        info.rate_of_fire  = extractFloat(dj, "rate_of_fire", 3.0f);
                        info.optimal_range = extractFloat(dj, "optimal_range", 5000.0f);
                        info.hitpoints     = extractFloat(dj, "hitpoints", 45.0f);
                        info.current_hp    = extractFloat(dj, "current_hp", 45.0f);
                        info.bandwidth_use = extractInt(dj, "bandwidth_use", 5);
                        info.volume        = extractFloat(dj, "volume", 5.0f);
                        out.push_back(info);
                        obj_start = std::string::npos;
                    }
                }
            }
        };

        parseDrones("stored",  db->stored_drones);
        parseDrones("deployed", db->deployed_drones);

        entity->addComponent(std::move(db));
    }

    // ContractBoard
    std::string cb_json = extractObject(json, "contract_board");
    if (!cb_json.empty()) {
        auto cb = std::make_unique<components::ContractBoard>();

        std::string contracts_key = "\"contracts\"";
        size_t ck_pos = cb_json.find(contracts_key);
        if (ck_pos != std::string::npos) {
            size_t arr_start = cb_json.find("[", ck_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < cb_json.size() && bracket_depth > 0) {
                    if (cb_json[arr_end] == '[') ++bracket_depth;
                    else if (cb_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = cb_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string cj = content.substr(obj_start, i - obj_start + 1);
                                components::ContractBoard::Contract contract;
                                contract.contract_id       = extractString(cj, "contract_id");
                                contract.issuer_id         = extractString(cj, "issuer_id");
                                contract.assignee_id       = extractString(cj, "assignee_id");
                                contract.type              = extractString(cj, "type");
                                contract.status            = extractString(cj, "status");
                                contract.isk_reward        = extractDouble(cj, "isk_reward", 0.0);
                                contract.isk_collateral    = extractDouble(cj, "isk_collateral", 0.0);
                                contract.duration_remaining = extractFloat(cj, "duration_remaining", -1.0f);
                                contract.days_to_complete  = extractFloat(cj, "days_to_complete", 3.0f);

                                auto parseItems = [&](const std::string& key,
                                                      std::vector<components::ContractBoard::ContractItem>& out) {
                                    std::string k = "\"" + key + "\"";
                                    size_t kp = cj.find(k);
                                    if (kp == std::string::npos) return;
                                    size_t as = cj.find("[", kp);
                                    if (as == std::string::npos) return;
                                    int bd = 1;
                                    size_t ae = as + 1;
                                    while (ae < cj.size() && bd > 0) {
                                        if (cj[ae] == '[') ++bd;
                                        else if (cj[ae] == ']') --bd;
                                        if (bd > 0) ++ae;
                                    }
                                    if (bd != 0) return;
                                    std::string ic = cj.substr(as + 1, ae - as - 1);
                                    int id2 = 0;
                                    size_t os2 = std::string::npos;
                                    for (size_t j = 0; j < ic.size(); ++j) {
                                        if (ic[j] == '{') {
                                            if (id2 == 0) os2 = j;
                                            ++id2;
                                        } else if (ic[j] == '}') {
                                            --id2;
                                            if (id2 == 0 && os2 != std::string::npos) {
                                                std::string ij = ic.substr(os2, j - os2 + 1);
                                                components::ContractBoard::ContractItem item;
                                                item.item_id  = extractString(ij, "item_id");
                                                item.name     = extractString(ij, "name");
                                                item.quantity = extractInt(ij, "quantity");
                                                item.volume   = extractFloat(ij, "volume", 1.0f);
                                                out.push_back(item);
                                                os2 = std::string::npos;
                                            }
                                        }
                                    }
                                };

                                parseItems("items_offered", contract.items_offered);
                                parseItems("items_requested", contract.items_requested);

                                cb->contracts.push_back(contract);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }

        entity->addComponent(std::move(cb));
    }

    // Station
    std::string sta_json = extractObject(json, "station");
    if (!sta_json.empty()) {
        auto sta = std::make_unique<components::Station>();
        sta->station_name      = extractString(sta_json, "station_name");
        sta->docking_range     = extractFloat(sta_json, "docking_range", 2500.0f);
        sta->repair_cost_per_hp = extractFloat(sta_json, "repair_cost_per_hp", 1.0f);
        sta->docked_count      = extractInt(sta_json, "docked_count");
        entity->addComponent(std::move(sta));
    }

    // Docked
    std::string dck_json = extractObject(json, "docked");
    if (!dck_json.empty()) {
        auto dck = std::make_unique<components::Docked>();
        dck->station_id = extractString(dck_json, "station_id");
        entity->addComponent(std::move(dck));
    }

    // Wreck
    std::string wrk_json = extractObject(json, "wreck");
    if (!wrk_json.empty()) {
        auto wrk = std::make_unique<components::Wreck>();
        wrk->source_entity_id   = extractString(wrk_json, "source_entity_id");
        wrk->lifetime_remaining = extractFloat(wrk_json, "lifetime_remaining", 1800.0f);
        wrk->salvaged           = extractBool(wrk_json, "salvaged", false);
        entity->addComponent(std::move(wrk));
    }

    // CaptainPersonality
    std::string cp_json = extractObject(json, "captain_personality");
    if (!cp_json.empty()) {
        auto cp = std::make_unique<components::CaptainPersonality>();
        cp->aggression       = extractFloat(cp_json, "aggression", 0.5f);
        cp->sociability      = extractFloat(cp_json, "sociability", 0.5f);
        cp->optimism         = extractFloat(cp_json, "optimism", 0.5f);
        cp->professionalism  = extractFloat(cp_json, "professionalism", 0.5f);
        cp->loyalty          = extractFloat(cp_json, "loyalty", 0.5f);
        cp->paranoia         = extractFloat(cp_json, "paranoia", 0.5f);
        cp->ambition         = extractFloat(cp_json, "ambition", 0.5f);
        cp->adaptability     = extractFloat(cp_json, "adaptability", 0.5f);
        cp->captain_name     = extractString(cp_json, "captain_name");
        cp->faction          = extractString(cp_json, "faction");
        entity->addComponent(std::move(cp));
    }

    // FleetMorale
    std::string fmor_json = extractObject(json, "fleet_morale");
    if (!fmor_json.empty()) {
        auto fmor = std::make_unique<components::FleetMorale>();
        fmor->morale_score          = extractFloat(fmor_json, "morale_score", 0.0f);
        fmor->wins                  = extractInt(fmor_json, "wins");
        fmor->losses                = extractInt(fmor_json, "losses");
        fmor->ships_lost            = extractInt(fmor_json, "ships_lost");
        fmor->times_saved_by_player = extractInt(fmor_json, "times_saved_by_player");
        fmor->times_player_saved    = extractInt(fmor_json, "times_player_saved");
        fmor->missions_together     = extractInt(fmor_json, "missions_together");
        fmor->morale_state          = extractString(fmor_json, "morale_state");
        if (fmor->morale_state.empty()) fmor->morale_state = "Steady";
        entity->addComponent(std::move(fmor));
    }

    // CaptainRelationship
    std::string cr_json = extractObject(json, "captain_relationship");
    if (!cr_json.empty()) {
        auto cr = std::make_unique<components::CaptainRelationship>();
        std::string rel_key = "\"relationships\"";
        size_t rel_pos = cr_json.find(rel_key);
        if (rel_pos != std::string::npos) {
            size_t arr_start = cr_json.find("[", rel_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < cr_json.size() && bracket_depth > 0) {
                    if (cr_json[arr_end] == '[') ++bracket_depth;
                    else if (cr_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = cr_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string rj = content.substr(obj_start, i - obj_start + 1);
                                components::CaptainRelationship::Relationship rel;
                                rel.other_captain_id = extractString(rj, "other_captain_id");
                                rel.affinity         = extractFloat(rj, "affinity", 0.0f);
                                cr->relationships.push_back(rel);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(cr));
    }

    // EmotionalState
    std::string es_json = extractObject(json, "emotional_state");
    if (!es_json.empty()) {
        auto es = std::make_unique<components::EmotionalState>();
        es->confidence      = extractFloat(es_json, "confidence", 50.0f);
        es->trust_in_player = extractFloat(es_json, "trust_in_player", 50.0f);
        es->fatigue         = extractFloat(es_json, "fatigue", 0.0f);
        es->hope            = extractFloat(es_json, "hope", 50.0f);
        entity->addComponent(std::move(es));
    }

    // CaptainMemory
    std::string cm_json = extractObject(json, "captain_memory");
    if (!cm_json.empty()) {
        auto cm = std::make_unique<components::CaptainMemory>();
        cm->max_memories = extractInt(cm_json, "max_memories", 50);
        std::string mem_key = "\"memories\"";
        size_t mem_pos = cm_json.find(mem_key);
        if (mem_pos != std::string::npos) {
            size_t arr_start = cm_json.find("[", mem_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < cm_json.size() && bracket_depth > 0) {
                    if (cm_json[arr_end] == '[') ++bracket_depth;
                    else if (cm_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = cm_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string mj = content.substr(obj_start, i - obj_start + 1);
                                components::CaptainMemory::MemoryEntry entry;
                                entry.event_type       = extractString(mj, "event_type");
                                entry.context          = extractString(mj, "context");
                                entry.timestamp        = extractFloat(mj, "timestamp", 0.0f);
                                entry.emotional_weight = extractFloat(mj, "emotional_weight", 0.0f);
                                cm->memories.push_back(entry);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(cm));
    }

    // FleetFormation
    std::string ff_json = extractObject(json, "fleet_formation");
    if (!ff_json.empty()) {
        auto ff = std::make_unique<components::FleetFormation>();
        int ft = extractInt(ff_json, "formation");
        ff->formation  = static_cast<components::FleetFormation::FormationType>(ft);
        ff->slot_index = extractInt(ff_json, "slot_index");
        ff->offset_x   = extractFloat(ff_json, "offset_x", 0.0f);
        ff->offset_y   = extractFloat(ff_json, "offset_y", 0.0f);
        ff->offset_z   = extractFloat(ff_json, "offset_z", 0.0f);
        ff->spacing_modifier = extractFloat(ff_json, "spacing_modifier", 1.0f);
        entity->addComponent(std::move(ff));
    }

    // FleetCargoPool
    std::string fcp_json = extractObject(json, "fleet_cargo_pool");
    if (!fcp_json.empty()) {
        auto fcp = std::make_unique<components::FleetCargoPool>();
        fcp->total_capacity = static_cast<uint64_t>(extractDouble(fcp_json, "total_capacity", 0.0));
        fcp->used_capacity  = static_cast<uint64_t>(extractDouble(fcp_json, "used_capacity", 0.0));

        std::string pi_json = extractObject(fcp_json, "pooled_items");
        if (!pi_json.empty()) {
            size_t pos = 0;
            while (pos < pi_json.size()) {
                size_t q1 = pi_json.find('\"', pos);
                if (q1 == std::string::npos) break;
                size_t q2 = pi_json.find('\"', q1 + 1);
                if (q2 == std::string::npos) break;
                std::string key = pi_json.substr(q1 + 1, q2 - q1 - 1);
                size_t colon = pi_json.find(':', q2 + 1);
                if (colon == std::string::npos) break;
                size_t vs = colon + 1;
                while (vs < pi_json.size() && pi_json[vs] == ' ') ++vs;
                size_t ve = vs;
                while (ve < pi_json.size() && (std::isdigit(pi_json[ve]) || pi_json[ve] == '.' || pi_json[ve] == '-' || pi_json[ve] == 'e' || pi_json[ve] == 'E' || pi_json[ve] == '+')) ++ve;
                if (ve > vs) {
                    uint64_t val = static_cast<uint64_t>(std::stod(pi_json.substr(vs, ve - vs)));
                    fcp->pooled_items[key] = val;
                }
                pos = ve;
            }
        }

        std::string cs_key = "\"contributor_ship_ids\"";
        size_t cs_pos = fcp_json.find(cs_key);
        if (cs_pos != std::string::npos) {
            size_t arr_start = fcp_json.find("[", cs_pos);
            size_t arr_end = fcp_json.find("]", arr_start);
            if (arr_start != std::string::npos && arr_end != std::string::npos) {
                std::string arr_content = fcp_json.substr(arr_start + 1, arr_end - arr_start - 1);
                size_t q1 = 0;
                while ((q1 = arr_content.find("\"", q1)) != std::string::npos) {
                    size_t q2 = arr_content.find("\"", q1 + 1);
                    if (q2 == std::string::npos) break;
                    fcp->contributor_ship_ids.push_back(arr_content.substr(q1 + 1, q2 - q1 - 1));
                    q1 = q2 + 1;
                }
            }
        }

        entity->addComponent(std::move(fcp));
    }

    // RumorLog
    std::string rl_json = extractObject(json, "rumor_log");
    if (!rl_json.empty()) {
        auto rl = std::make_unique<components::RumorLog>();
        std::string ru_key = "\"rumors\"";
        size_t ru_pos = rl_json.find(ru_key);
        if (ru_pos != std::string::npos) {
            size_t arr_start = rl_json.find("[", ru_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < rl_json.size() && bracket_depth > 0) {
                    if (rl_json[arr_end] == '[') ++bracket_depth;
                    else if (rl_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = rl_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string rj = content.substr(obj_start, i - obj_start + 1);
                                components::RumorLog::Rumor rumor;
                                rumor.rumor_id             = extractString(rj, "rumor_id");
                                rumor.text                 = extractString(rj, "text");
                                rumor.belief_strength      = extractFloat(rj, "belief_strength", 0.5f);
                                rumor.personally_witnessed = extractBool(rj, "personally_witnessed", false);
                                rumor.times_heard          = extractInt(rj, "times_heard");
                                rl->rumors.push_back(rumor);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(rl));
    }

    // MineralDeposit
    std::string md_json = extractObject(json, "mineral_deposit");
    if (!md_json.empty()) {
        auto md = std::make_unique<components::MineralDeposit>();
        md->mineral_type       = extractString(md_json, "mineral_type");
        if (md->mineral_type.empty()) md->mineral_type = "Ferrite";
        md->quantity_remaining = extractFloat(md_json, "quantity_remaining", 10000.0f);
        md->max_quantity       = extractFloat(md_json, "max_quantity", 10000.0f);
        md->yield_rate         = extractFloat(md_json, "yield_rate", 1.0f);
        md->volume_per_unit    = extractFloat(md_json, "volume_per_unit", 0.1f);
        entity->addComponent(std::move(md));
    }

    // SystemResources
    std::string sr_json = extractObject(json, "system_resources");
    if (!sr_json.empty()) {
        auto sr = std::make_unique<components::SystemResources>();
        std::string res_key = "\"resources\"";
        size_t res_pos = sr_json.find(res_key);
        if (res_pos != std::string::npos) {
            size_t arr_start = sr_json.find("[", res_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < sr_json.size() && bracket_depth > 0) {
                    if (sr_json[arr_end] == '[') ++bracket_depth;
                    else if (sr_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = sr_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string rj = content.substr(obj_start, i - obj_start + 1);
                                components::SystemResources::ResourceEntry entry;
                                entry.mineral_type       = extractString(rj, "mineral_type");
                                entry.total_quantity     = extractFloat(rj, "total_quantity", 0.0f);
                                entry.remaining_quantity = extractFloat(rj, "remaining_quantity", 0.0f);
                                sr->resources.push_back(entry);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(sr));
    }

    // MarketHub
    std::string mh_json = extractObject(json, "market_hub");
    if (!mh_json.empty()) {
        auto mh = std::make_unique<components::MarketHub>();
        mh->station_id     = extractString(mh_json, "station_id");
        mh->broker_fee_rate = extractDouble(mh_json, "broker_fee_rate", 0.02);
        mh->sales_tax_rate  = extractDouble(mh_json, "sales_tax_rate", 0.04);

        std::string ord_key = "\"orders\"";
        size_t ord_pos = mh_json.find(ord_key);
        if (ord_pos != std::string::npos) {
            size_t arr_start = mh_json.find("[", ord_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < mh_json.size() && bracket_depth > 0) {
                    if (mh_json[arr_end] == '[') ++bracket_depth;
                    else if (mh_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = mh_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string oj = content.substr(obj_start, i - obj_start + 1);
                                components::MarketHub::Order order;
                                order.order_id           = extractString(oj, "order_id");
                                order.item_id            = extractString(oj, "item_id");
                                order.item_name          = extractString(oj, "item_name");
                                order.owner_id           = extractString(oj, "owner_id");
                                order.is_buy_order       = extractBool(oj, "is_buy_order", false);
                                order.price_per_unit     = extractDouble(oj, "price_per_unit", 0.0);
                                order.quantity           = extractInt(oj, "quantity", 1);
                                order.quantity_remaining = extractInt(oj, "quantity_remaining", 1);
                                order.duration_remaining = extractFloat(oj, "duration_remaining", -1.0f);
                                order.fulfilled          = extractBool(oj, "fulfilled", false);
                                mh->orders.push_back(order);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }

        entity->addComponent(std::move(mh));
    }

    // AnomalyVisualCue
    std::string avc_json = extractObject(json, "anomaly_visual_cue");
    if (!avc_json.empty()) {
        auto avc = std::make_unique<components::AnomalyVisualCue>();
        avc->anomaly_id = extractString(avc_json, "anomaly_id");
        avc->cue_type = static_cast<components::AnomalyVisualCue::CueType>(
            extractInt(avc_json, "cue_type", 5));
        avc->intensity = extractFloat(avc_json, "intensity");
        avc->radius = extractFloat(avc_json, "radius");
        avc->pulse_frequency = extractFloat(avc_json, "pulse_frequency");
        avc->r = extractFloat(avc_json, "r");
        avc->g = extractFloat(avc_json, "g");
        avc->b = extractFloat(avc_json, "b");
        avc->distortion_strength = extractFloat(avc_json, "distortion_strength");
        avc->active = extractBool(avc_json, "active", true);
        entity->addComponent(std::move(avc));
    }

    // LODPriority
    std::string lod_json = extractObject(json, "lod_priority");
    if (!lod_json.empty()) {
        auto lod = std::make_unique<components::LODPriority>();
        lod->priority = extractFloat(lod_json, "priority");
        lod->force_visible = extractBool(lod_json, "force_visible", false);
        lod->impostor_distance = extractFloat(lod_json, "impostor_distance");
        entity->addComponent(std::move(lod));
    }

    // WarpProfile
    std::string wp_json = extractObject(json, "warp_profile");
    if (!wp_json.empty()) {
        auto wp = std::make_unique<components::WarpProfile>();
        wp->warp_speed = extractFloat(wp_json, "warp_speed");
        wp->mass_norm = extractFloat(wp_json, "mass_norm");
        wp->intensity = extractFloat(wp_json, "intensity");
        wp->comfort_scale = extractFloat(wp_json, "comfort_scale");
        entity->addComponent(std::move(wp));
    }

    // WarpVisual
    std::string wv_json = extractObject(json, "warp_visual");
    if (!wv_json.empty()) {
        auto wv = std::make_unique<components::WarpVisual>();
        wv->distortion_strength = extractFloat(wv_json, "distortion_strength");
        wv->tunnel_noise_scale = extractFloat(wv_json, "tunnel_noise_scale");
        wv->vignette_amount = extractFloat(wv_json, "vignette_amount");
        wv->bloom_strength = extractFloat(wv_json, "bloom_strength");
        wv->starfield_speed = extractFloat(wv_json, "starfield_speed");
        entity->addComponent(std::move(wv));
    }

    // WarpEvent
    std::string we_json = extractObject(json, "warp_event");
    if (!we_json.empty()) {
        auto we = std::make_unique<components::WarpEvent>();
        we->current_event = extractString(we_json, "current_event");
        we->event_timer = extractFloat(we_json, "event_timer");
        we->severity = extractInt(we_json, "severity", 0);
        entity->addComponent(std::move(we));
    }

    // TacticalProjection
    std::string tp_json = extractObject(json, "tactical_projection");
    if (!tp_json.empty()) {
        auto tp = std::make_unique<components::TacticalProjection>();
        tp->projected_x = extractFloat(tp_json, "projected_x");
        tp->projected_y = extractFloat(tp_json, "projected_y");
        tp->vertical_offset = extractFloat(tp_json, "vertical_offset");
        tp->visible = extractBool(tp_json, "visible", true);
        entity->addComponent(std::move(tp));
    }

    // PlayerPresence
    std::string pp_json = extractObject(json, "player_presence");
    if (!pp_json.empty()) {
        auto pp = std::make_unique<components::PlayerPresence>();
        pp->time_since_last_command = extractFloat(pp_json, "time_since_last_command");
        pp->time_since_last_speech = extractFloat(pp_json, "time_since_last_speech");
        entity->addComponent(std::move(pp));
    }

    // FactionCulture
    std::string fc_json = extractObject(json, "faction_culture");
    if (!fc_json.empty()) {
        auto fc = std::make_unique<components::FactionCulture>();
        fc->faction = extractString(fc_json, "faction");
        fc->chatter_frequency_mod = extractFloat(fc_json, "chatter_frequency_mod");
        fc->formation_tightness_mod = extractFloat(fc_json, "formation_tightness_mod");
        fc->morale_sensitivity = extractFloat(fc_json, "morale_sensitivity");
        fc->risk_tolerance = extractFloat(fc_json, "risk_tolerance");
        entity->addComponent(std::move(fc));
    }

    return true;
}

} // namespace data
} // namespace atlas
