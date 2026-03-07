#include "data/world_persistence.h"
#include "components/game_components.h"
#include <sstream>

namespace atlas {
namespace data {

// Escape a string for safe JSON embedding
static std::string escapeJson(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Skip other control characters
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

std::string WorldPersistence::serializeWorld(const ecs::World* world) const {
    // We need a non-const World* to call getAllEntities (existing API limitation)
    auto* mutable_world = const_cast<ecs::World*>(world);
    auto entities = mutable_world->getAllEntities();

    std::ostringstream json;
    json << "{\"entities\":[";

    bool first = true;
    for (const auto* entity : entities) {
        if (!first) json << ",";
        first = false;
        json << serializeEntity(entity);
    }

    json << "]}";
    return json.str();
}

// ---------------------------------------------------------------------------
// Single-entity serialization
// ---------------------------------------------------------------------------

std::string WorldPersistence::serializeEntity(
        const ecs::Entity* entity) const {
    std::ostringstream json;
    json << "{\"id\":\"" << escapeJson(entity->getId()) << "\"";

    // Position
    auto* pos = entity->getComponent<components::Position>();
    if (pos) {
        json << ",\"position\":{\"x\":" << pos->x
             << ",\"y\":" << pos->y
             << ",\"z\":" << pos->z
             << ",\"rotation\":" << pos->rotation << "}";
    }

    // Velocity
    auto* vel = entity->getComponent<components::Velocity>();
    if (vel) {
        json << ",\"velocity\":{\"vx\":" << vel->vx
             << ",\"vy\":" << vel->vy
             << ",\"vz\":" << vel->vz
             << ",\"angular_velocity\":" << vel->angular_velocity
             << ",\"max_speed\":" << vel->max_speed << "}";
    }

    // Health
    auto* hp = entity->getComponent<components::Health>();
    if (hp) {
        json << ",\"health\":{"
             << "\"hull_hp\":" << hp->hull_hp
             << ",\"hull_max\":" << hp->hull_max
             << ",\"armor_hp\":" << hp->armor_hp
             << ",\"armor_max\":" << hp->armor_max
             << ",\"shield_hp\":" << hp->shield_hp
             << ",\"shield_max\":" << hp->shield_max
             << ",\"shield_recharge_rate\":" << hp->shield_recharge_rate
             << ",\"hull_em_resist\":" << hp->hull_em_resist
             << ",\"hull_thermal_resist\":" << hp->hull_thermal_resist
             << ",\"hull_kinetic_resist\":" << hp->hull_kinetic_resist
             << ",\"hull_explosive_resist\":" << hp->hull_explosive_resist
             << ",\"armor_em_resist\":" << hp->armor_em_resist
             << ",\"armor_thermal_resist\":" << hp->armor_thermal_resist
             << ",\"armor_kinetic_resist\":" << hp->armor_kinetic_resist
             << ",\"armor_explosive_resist\":" << hp->armor_explosive_resist
             << ",\"shield_em_resist\":" << hp->shield_em_resist
             << ",\"shield_thermal_resist\":" << hp->shield_thermal_resist
             << ",\"shield_kinetic_resist\":" << hp->shield_kinetic_resist
             << ",\"shield_explosive_resist\":" << hp->shield_explosive_resist
             << "}";
    }

    // Capacitor
    auto* cap = entity->getComponent<components::Capacitor>();
    if (cap) {
        json << ",\"capacitor\":{"
             << "\"capacitor\":" << cap->capacitor
             << ",\"capacitor_max\":" << cap->capacitor_max
             << ",\"recharge_rate\":" << cap->recharge_rate << "}";
    }

    // Ship
    auto* ship = entity->getComponent<components::Ship>();
    if (ship) {
        json << ",\"ship\":{"
             << "\"ship_type\":\"" << escapeJson(ship->ship_type) << "\""
             << ",\"ship_class\":\"" << escapeJson(ship->ship_class) << "\""
             << ",\"ship_name\":\"" << escapeJson(ship->ship_name) << "\""
             << ",\"race\":\"" << escapeJson(ship->race) << "\""
             << ",\"cpu\":" << ship->cpu
             << ",\"cpu_max\":" << ship->cpu_max
             << ",\"powergrid\":" << ship->powergrid
             << ",\"powergrid_max\":" << ship->powergrid_max
             << ",\"signature_radius\":" << ship->signature_radius
             << ",\"scan_resolution\":" << ship->scan_resolution
             << ",\"max_locked_targets\":" << ship->max_locked_targets
             << ",\"max_targeting_range\":" << ship->max_targeting_range
             << "}";
    }

    // Faction
    auto* fac = entity->getComponent<components::Faction>();
    if (fac) {
        json << ",\"faction\":{"
             << "\"faction_name\":\"" << escapeJson(fac->faction_name) << "\""
             << "}";
    }

    // Standings
    auto* standings = entity->getComponent<components::Standings>();
    if (standings) {
        json << ",\"standings\":{";
        
        // Serialize personal standings
        if (!standings->personal_standings.empty()) {
            json << "\"personal\":{";
            bool first = true;
            for (const auto& [entity_id, standing] : standings->personal_standings) {
                if (!first) json << ",";
                json << "\"" << escapeJson(entity_id) << "\":" << standing;
                first = false;
            }
            json << "}";
        }
        
        // Serialize corporation standings
        if (!standings->corporation_standings.empty()) {
            if (!standings->personal_standings.empty()) json << ",";
            json << "\"corporation\":{";
            bool first = true;
            for (const auto& [corp_name, standing] : standings->corporation_standings) {
                if (!first) json << ",";
                json << "\"" << escapeJson(corp_name) << "\":" << standing;
                first = false;
            }
            json << "}";
        }
        
        // Serialize faction standings
        if (!standings->faction_standings.empty()) {
            if (!standings->personal_standings.empty() || !standings->corporation_standings.empty()) {
                json << ",";
            }
            json << "\"faction\":{";
            bool first = true;
            for (const auto& [faction_name, standing] : standings->faction_standings) {
                if (!first) json << ",";
                json << "\"" << escapeJson(faction_name) << "\":" << standing;
                first = false;
            }
            json << "}";
        }
        
        json << "}";
    }

    // AI
    auto* ai = entity->getComponent<components::AI>();
    if (ai) {
        json << ",\"ai\":{"
             << "\"behavior\":" << static_cast<int>(ai->behavior)
             << ",\"state\":" << static_cast<int>(ai->state)
             << ",\"target_entity_id\":\"" << escapeJson(ai->target_entity_id) << "\""
             << ",\"orbit_distance\":" << ai->orbit_distance
             << ",\"awareness_range\":" << ai->awareness_range
             << "}";
    }

    // Weapon
    auto* weapon = entity->getComponent<components::Weapon>();
    if (weapon) {
        json << ",\"weapon\":{"
             << "\"weapon_type\":\"" << escapeJson(weapon->weapon_type) << "\""
             << ",\"damage_type\":\"" << escapeJson(weapon->damage_type) << "\""
             << ",\"damage\":" << weapon->damage
             << ",\"optimal_range\":" << weapon->optimal_range
             << ",\"falloff_range\":" << weapon->falloff_range
             << ",\"tracking_speed\":" << weapon->tracking_speed
             << ",\"rate_of_fire\":" << weapon->rate_of_fire
             << ",\"capacitor_cost\":" << weapon->capacitor_cost
             << ",\"ammo_type\":\"" << escapeJson(weapon->ammo_type) << "\""
             << ",\"ammo_count\":" << weapon->ammo_count
             << "}";
    }

    // Player
    auto* player = entity->getComponent<components::Player>();
    if (player) {
        json << ",\"player\":{"
             << "\"player_id\":\"" << escapeJson(player->player_id) << "\""
             << ",\"character_name\":\"" << escapeJson(player->character_name) << "\""
             << ",\"credits\":" << player->credits
             << ",\"corporation\":\"" << escapeJson(player->corporation) << "\""
             << "}";
    }

    // WormholeConnection
    auto* wh = entity->getComponent<components::WormholeConnection>();
    if (wh) {
        json << ",\"wormhole_connection\":{"
             << "\"wormhole_id\":\"" << escapeJson(wh->wormhole_id) << "\""
             << ",\"source_system\":\"" << escapeJson(wh->source_system) << "\""
             << ",\"destination_system\":\"" << escapeJson(wh->destination_system) << "\""
             << ",\"max_mass\":" << wh->max_mass
             << ",\"remaining_mass\":" << wh->remaining_mass
             << ",\"max_jump_mass\":" << wh->max_jump_mass
             << ",\"max_lifetime_hours\":" << wh->max_lifetime_hours
             << ",\"elapsed_hours\":" << wh->elapsed_hours
             << ",\"collapsed\":" << (wh->collapsed ? "true" : "false")
             << "}";
    }

    // SolarSystem
    auto* ss = entity->getComponent<components::SolarSystem>();
    if (ss) {
        json << ",\"solar_system\":{"
             << "\"system_id\":\"" << escapeJson(ss->system_id) << "\""
             << ",\"system_name\":\"" << escapeJson(ss->system_name) << "\""
             << ",\"wormhole_class\":" << ss->wormhole_class
             << ",\"effect_name\":\"" << escapeJson(ss->effect_name) << "\""
             << ",\"dormants_spawned\":" << (ss->dormants_spawned ? "true" : "false")
             << "}";
    }

    // FleetMembership
    auto* fm = entity->getComponent<components::FleetMembership>();
    if (fm) {
        json << ",\"fleet_membership\":{"
             << "\"fleet_id\":\"" << escapeJson(fm->fleet_id) << "\""
             << ",\"role\":\"" << escapeJson(fm->role) << "\""
             << ",\"squad_id\":\"" << escapeJson(fm->squad_id) << "\""
             << ",\"wing_id\":\"" << escapeJson(fm->wing_id) << "\""
             << "}";
    }

    // Inventory
    auto* inv = entity->getComponent<components::Inventory>();
    if (inv) {
        json << ",\"inventory\":{"
             << "\"max_capacity\":" << inv->max_capacity
             << ",\"items\":[";
        bool first_item = true;
        for (const auto& item : inv->items) {
            if (!first_item) json << ",";
            first_item = false;
            json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                 << ",\"name\":\"" << escapeJson(item.name) << "\""
                 << ",\"type\":\"" << escapeJson(item.type) << "\""
                 << ",\"quantity\":" << item.quantity
                 << ",\"volume\":" << item.volume << "}";
        }
        json << "]}";
    }

    // LootTable
    auto* lt = entity->getComponent<components::LootTable>();
    if (lt) {
        json << ",\"loot_table\":{"
             << "\"isc_drop\":" << lt->isc_drop
             << ",\"entries\":[";
        bool first_entry = true;
        for (const auto& entry : lt->entries) {
            if (!first_entry) json << ",";
            first_entry = false;
            json << "{\"item_id\":\"" << escapeJson(entry.item_id) << "\""
                 << ",\"name\":\"" << escapeJson(entry.name) << "\""
                 << ",\"type\":\"" << escapeJson(entry.type) << "\""
                 << ",\"drop_chance\":" << entry.drop_chance
                 << ",\"min_quantity\":" << entry.min_quantity
                 << ",\"max_quantity\":" << entry.max_quantity
                 << ",\"volume\":" << entry.volume << "}";
        }
        json << "]}";
    }

    // Corporation
    auto* corp = entity->getComponent<components::Corporation>();
    if (corp) {
        json << ",\"corporation_data\":{"
             << "\"corp_id\":\"" << escapeJson(corp->corp_id) << "\""
             << ",\"corp_name\":\"" << escapeJson(corp->corp_name) << "\""
             << ",\"ticker\":\"" << escapeJson(corp->ticker) << "\""
             << ",\"ceo_id\":\"" << escapeJson(corp->ceo_id) << "\""
             << ",\"tax_rate\":" << corp->tax_rate
             << ",\"corp_wallet\":" << corp->corp_wallet
             << ",\"member_ids\":[";
        bool first_mid = true;
        for (const auto& mid : corp->member_ids) {
            if (!first_mid) json << ",";
            first_mid = false;
            json << "\"" << escapeJson(mid) << "\"";
        }
        json << "],\"hangar_items\":[";
        bool first_hi = true;
        for (const auto& item : corp->hangar_items) {
            if (!first_hi) json << ",";
            first_hi = false;
            json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                 << ",\"name\":\"" << escapeJson(item.name) << "\""
                 << ",\"type\":\"" << escapeJson(item.type) << "\""
                 << ",\"quantity\":" << item.quantity
                 << ",\"volume\":" << item.volume << "}";
        }
        json << "]}";
    }

    // DroneBay
    auto* db = entity->getComponent<components::DroneBay>();
    if (db) {
        json << ",\"drone_bay\":{"
             << "\"bay_capacity\":" << db->bay_capacity
             << ",\"max_bandwidth\":" << db->max_bandwidth
             << ",\"stored\":[";
        bool first_s = true;
        for (const auto& d : db->stored_drones) {
            if (!first_s) json << ",";
            first_s = false;
            json << "{\"drone_id\":\"" << escapeJson(d.drone_id) << "\""
                 << ",\"name\":\"" << escapeJson(d.name) << "\""
                 << ",\"type\":\"" << escapeJson(d.type) << "\""
                 << ",\"damage_type\":\"" << escapeJson(d.damage_type) << "\""
                 << ",\"damage\":" << d.damage
                 << ",\"rate_of_fire\":" << d.rate_of_fire
                 << ",\"optimal_range\":" << d.optimal_range
                 << ",\"hitpoints\":" << d.hitpoints
                 << ",\"current_hp\":" << d.current_hp
                 << ",\"bandwidth_use\":" << d.bandwidth_use
                 << ",\"volume\":" << d.volume << "}";
        }
        json << "],\"deployed\":[";
        bool first_d = true;
        for (const auto& d : db->deployed_drones) {
            if (!first_d) json << ",";
            first_d = false;
            json << "{\"drone_id\":\"" << escapeJson(d.drone_id) << "\""
                 << ",\"name\":\"" << escapeJson(d.name) << "\""
                 << ",\"type\":\"" << escapeJson(d.type) << "\""
                 << ",\"damage_type\":\"" << escapeJson(d.damage_type) << "\""
                 << ",\"damage\":" << d.damage
                 << ",\"rate_of_fire\":" << d.rate_of_fire
                 << ",\"optimal_range\":" << d.optimal_range
                 << ",\"hitpoints\":" << d.hitpoints
                 << ",\"current_hp\":" << d.current_hp
                 << ",\"bandwidth_use\":" << d.bandwidth_use
                 << ",\"volume\":" << d.volume << "}";
        }
        json << "]}";
    }

    // ContractBoard
    auto* cb = entity->getComponent<components::ContractBoard>();
    if (cb) {
        json << ",\"contract_board\":{\"contracts\":[";
        bool first_c = true;
        for (const auto& c : cb->contracts) {
            if (!first_c) json << ",";
            first_c = false;
            json << "{\"contract_id\":\"" << escapeJson(c.contract_id) << "\""
                 << ",\"issuer_id\":\"" << escapeJson(c.issuer_id) << "\""
                 << ",\"assignee_id\":\"" << escapeJson(c.assignee_id) << "\""
                 << ",\"type\":\"" << escapeJson(c.type) << "\""
                 << ",\"status\":\"" << escapeJson(c.status) << "\""
                 << ",\"isc_reward\":" << c.isc_reward
                 << ",\"isc_collateral\":" << c.isc_collateral
                 << ",\"duration_remaining\":" << c.duration_remaining
                 << ",\"days_to_complete\":" << c.days_to_complete
                 << ",\"items_offered\":[";
            bool first_io = true;
            for (const auto& item : c.items_offered) {
                if (!first_io) json << ",";
                first_io = false;
                json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                     << ",\"name\":\"" << escapeJson(item.name) << "\""
                     << ",\"quantity\":" << item.quantity
                     << ",\"volume\":" << item.volume << "}";
            }
            json << "],\"items_requested\":[";
            bool first_ir = true;
            for (const auto& item : c.items_requested) {
                if (!first_ir) json << ",";
                first_ir = false;
                json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                     << ",\"name\":\"" << escapeJson(item.name) << "\""
                     << ",\"quantity\":" << item.quantity
                     << ",\"volume\":" << item.volume << "}";
            }
            json << "]}";
        }
        json << "]}";
    }

    // Station
    auto* sta = entity->getComponent<components::Station>();
    if (sta) {
        json << ",\"station\":{"
             << "\"station_name\":\"" << escapeJson(sta->station_name) << "\""
             << ",\"docking_range\":" << sta->docking_range
             << ",\"repair_cost_per_hp\":" << sta->repair_cost_per_hp
             << ",\"docked_count\":" << sta->docked_count
             << "}";
    }

    // Docked
    auto* dck = entity->getComponent<components::Docked>();
    if (dck) {
        json << ",\"docked\":{"
             << "\"station_id\":\"" << escapeJson(dck->station_id) << "\""
             << "}";
    }

    // Wreck
    auto* wrk = entity->getComponent<components::Wreck>();
    if (wrk) {
        json << ",\"wreck\":{"
             << "\"source_entity_id\":\"" << escapeJson(wrk->source_entity_id) << "\""
             << ",\"lifetime_remaining\":" << wrk->lifetime_remaining
             << ",\"salvaged\":" << (wrk->salvaged ? "true" : "false")
             << "}";
    }

    // CaptainPersonality
    auto* cp = entity->getComponent<components::CaptainPersonality>();
    if (cp) {
        json << ",\"captain_personality\":{"
             << "\"aggression\":" << cp->aggression
             << ",\"sociability\":" << cp->sociability
             << ",\"optimism\":" << cp->optimism
             << ",\"professionalism\":" << cp->professionalism
             << ",\"loyalty\":" << cp->loyalty
             << ",\"paranoia\":" << cp->paranoia
             << ",\"ambition\":" << cp->ambition
             << ",\"adaptability\":" << cp->adaptability
             << ",\"captain_name\":\"" << escapeJson(cp->captain_name) << "\""
             << ",\"faction\":\"" << escapeJson(cp->faction) << "\""
             << "}";
    }

    // FleetMorale
    auto* fmor = entity->getComponent<components::FleetMorale>();
    if (fmor) {
        json << ",\"fleet_morale\":{"
             << "\"morale_score\":" << fmor->morale_score
             << ",\"wins\":" << fmor->wins
             << ",\"losses\":" << fmor->losses
             << ",\"ships_lost\":" << fmor->ships_lost
             << ",\"times_saved_by_player\":" << fmor->times_saved_by_player
             << ",\"times_player_saved\":" << fmor->times_player_saved
             << ",\"missions_together\":" << fmor->missions_together
             << ",\"morale_state\":\"" << escapeJson(fmor->morale_state) << "\""
             << "}";
    }

    // CaptainRelationship
    auto* cr = entity->getComponent<components::CaptainRelationship>();
    if (cr) {
        json << ",\"captain_relationship\":{\"relationships\":[";
        bool first_r = true;
        for (const auto& r : cr->relationships) {
            if (!first_r) json << ",";
            first_r = false;
            json << "{\"other_captain_id\":\"" << escapeJson(r.other_captain_id) << "\""
                 << ",\"affinity\":" << r.affinity << "}";
        }
        json << "]}";
    }

    // EmotionalState
    auto* es = entity->getComponent<components::EmotionalState>();
    if (es) {
        json << ",\"emotional_state\":{"
             << "\"confidence\":" << es->confidence
             << ",\"trust_in_player\":" << es->trust_in_player
             << ",\"fatigue\":" << es->fatigue
             << ",\"hope\":" << es->hope
             << "}";
    }

    // CaptainMemory
    auto* cm = entity->getComponent<components::CaptainMemory>();
    if (cm) {
        json << ",\"captain_memory\":{\"max_memories\":" << cm->max_memories
             << ",\"memories\":[";
        bool first_m = true;
        for (const auto& m : cm->memories) {
            if (!first_m) json << ",";
            first_m = false;
            json << "{\"event_type\":\"" << escapeJson(m.event_type) << "\""
                 << ",\"context\":\"" << escapeJson(m.context) << "\""
                 << ",\"timestamp\":" << m.timestamp
                 << ",\"emotional_weight\":" << m.emotional_weight << "}";
        }
        json << "]}";
    }

    // FleetFormation
    auto* ff = entity->getComponent<components::FleetFormation>();
    if (ff) {
        json << ",\"fleet_formation\":{"
             << "\"formation\":" << static_cast<int>(ff->formation)
             << ",\"slot_index\":" << ff->slot_index
             << ",\"offset_x\":" << ff->offset_x
             << ",\"offset_y\":" << ff->offset_y
             << ",\"offset_z\":" << ff->offset_z
             << ",\"spacing_modifier\":" << ff->spacing_modifier
             << "}";
    }

    // FleetCargoPool
    auto* fcp = entity->getComponent<components::FleetCargoPool>();
    if (fcp) {
        json << ",\"fleet_cargo_pool\":{"
             << "\"total_capacity\":" << fcp->total_capacity
             << ",\"used_capacity\":" << fcp->used_capacity
             << ",\"pooled_items\":{";
        bool first_pi = true;
        for (const auto& [item_type, qty] : fcp->pooled_items) {
            if (!first_pi) json << ",";
            first_pi = false;
            json << "\"" << escapeJson(item_type) << "\":" << qty;
        }
        json << "},\"contributor_ship_ids\":[";
        bool first_cs = true;
        for (const auto& sid : fcp->contributor_ship_ids) {
            if (!first_cs) json << ",";
            first_cs = false;
            json << "\"" << escapeJson(sid) << "\"";
        }
        json << "]}";
    }

    // RumorLog
    auto* rl = entity->getComponent<components::RumorLog>();
    if (rl) {
        json << ",\"rumor_log\":{\"rumors\":[";
        bool first_ru = true;
        for (const auto& r : rl->rumors) {
            if (!first_ru) json << ",";
            first_ru = false;
            json << "{\"rumor_id\":\"" << escapeJson(r.rumor_id) << "\""
                 << ",\"text\":\"" << escapeJson(r.text) << "\""
                 << ",\"belief_strength\":" << r.belief_strength
                 << ",\"personally_witnessed\":" << (r.personally_witnessed ? "true" : "false")
                 << ",\"times_heard\":" << r.times_heard << "}";
        }
        json << "]}";
    }

    // MineralDeposit
    auto* md = entity->getComponent<components::MineralDeposit>();
    if (md) {
        json << ",\"mineral_deposit\":{"
             << "\"mineral_type\":\"" << escapeJson(md->mineral_type) << "\""
             << ",\"quantity_remaining\":" << md->quantity_remaining
             << ",\"max_quantity\":" << md->max_quantity
             << ",\"yield_rate\":" << md->yield_rate
             << ",\"volume_per_unit\":" << md->volume_per_unit
             << "}";
    }

    // SystemResources
    auto* sr = entity->getComponent<components::SystemResources>();
    if (sr) {
        json << ",\"system_resources\":{\"resources\":[";
        bool first_sr = true;
        for (const auto& r : sr->resources) {
            if (!first_sr) json << ",";
            first_sr = false;
            json << "{\"mineral_type\":\"" << escapeJson(r.mineral_type) << "\""
                 << ",\"total_quantity\":" << r.total_quantity
                 << ",\"remaining_quantity\":" << r.remaining_quantity << "}";
        }
        json << "]}";
    }

    // MarketHub
    auto* mh = entity->getComponent<components::MarketHub>();
    if (mh) {
        json << ",\"market_hub\":{"
             << "\"station_id\":\"" << escapeJson(mh->station_id) << "\""
             << ",\"broker_fee_rate\":" << mh->broker_fee_rate
             << ",\"sales_tax_rate\":" << mh->sales_tax_rate
             << ",\"orders\":[";
        bool first_o = true;
        for (const auto& o : mh->orders) {
            if (!first_o) json << ",";
            first_o = false;
            json << "{\"order_id\":\"" << escapeJson(o.order_id) << "\""
                 << ",\"item_id\":\"" << escapeJson(o.item_id) << "\""
                 << ",\"item_name\":\"" << escapeJson(o.item_name) << "\""
                 << ",\"owner_id\":\"" << escapeJson(o.owner_id) << "\""
                 << ",\"is_buy_order\":" << (o.is_buy_order ? "true" : "false")
                 << ",\"price_per_unit\":" << o.price_per_unit
                 << ",\"quantity\":" << o.quantity
                 << ",\"quantity_remaining\":" << o.quantity_remaining
                 << ",\"duration_remaining\":" << o.duration_remaining
                 << ",\"fulfilled\":" << (o.fulfilled ? "true" : "false") << "}";
        }
        json << "]}";
    }

    // AnomalyVisualCue
    auto* avc = entity->getComponent<components::AnomalyVisualCue>();
    if (avc) {
        json << ",\"anomaly_visual_cue\":{"
             << "\"anomaly_id\":\"" << escapeJson(avc->anomaly_id) << "\""
             << ",\"cue_type\":" << static_cast<int>(avc->cue_type)
             << ",\"intensity\":" << avc->intensity
             << ",\"radius\":" << avc->radius
             << ",\"pulse_frequency\":" << avc->pulse_frequency
             << ",\"r\":" << avc->r
             << ",\"g\":" << avc->g
             << ",\"b\":" << avc->b
             << ",\"distortion_strength\":" << avc->distortion_strength
             << ",\"active\":" << (avc->active ? "true" : "false") << "}";
    }

    // LODPriority
    auto* lod = entity->getComponent<components::LODPriority>();
    if (lod) {
        json << ",\"lod_priority\":{"
             << "\"priority\":" << lod->priority
             << ",\"force_visible\":" << (lod->force_visible ? "true" : "false")
             << ",\"impostor_distance\":" << lod->impostor_distance << "}";
    }

    // WarpProfile
    auto* wp = entity->getComponent<components::WarpProfile>();
    if (wp) {
        json << ",\"warp_profile\":{"
             << "\"warp_speed\":" << wp->warp_speed
             << ",\"mass_norm\":" << wp->mass_norm
             << ",\"intensity\":" << wp->intensity
             << ",\"comfort_scale\":" << wp->comfort_scale << "}";
    }

    // WarpVisual
    auto* wv = entity->getComponent<components::WarpVisual>();
    if (wv) {
        json << ",\"warp_visual\":{"
             << "\"distortion_strength\":" << wv->distortion_strength
             << ",\"tunnel_noise_scale\":" << wv->tunnel_noise_scale
             << ",\"vignette_amount\":" << wv->vignette_amount
             << ",\"bloom_strength\":" << wv->bloom_strength
             << ",\"starfield_speed\":" << wv->starfield_speed << "}";
    }

    // WarpEvent
    auto* we = entity->getComponent<components::WarpEvent>();
    if (we) {
        json << ",\"warp_event\":{"
             << "\"current_event\":\"" << we->current_event << "\""
             << ",\"event_timer\":" << we->event_timer
             << ",\"severity\":" << we->severity << "}";
    }

    // TacticalProjection
    auto* tp = entity->getComponent<components::TacticalProjection>();
    if (tp) {
        json << ",\"tactical_projection\":{"
             << "\"projected_x\":" << tp->projected_x
             << ",\"projected_y\":" << tp->projected_y
             << ",\"vertical_offset\":" << tp->vertical_offset
             << ",\"visible\":" << (tp->visible ? "true" : "false") << "}";
    }

    // PlayerPresence
    auto* pp = entity->getComponent<components::PlayerPresence>();
    if (pp) {
        json << ",\"player_presence\":{"
             << "\"time_since_last_command\":" << pp->time_since_last_command
             << ",\"time_since_last_speech\":" << pp->time_since_last_speech << "}";
    }

    // FactionCulture
    auto* fc = entity->getComponent<components::FactionCulture>();
    if (fc) {
        json << ",\"faction_culture\":{"
             << "\"faction\":\"" << fc->faction << "\""
             << ",\"chatter_frequency_mod\":" << fc->chatter_frequency_mod
             << ",\"formation_tightness_mod\":" << fc->formation_tightness_mod
             << ",\"morale_sensitivity\":" << fc->morale_sensitivity
             << ",\"risk_tolerance\":" << fc->risk_tolerance << "}";
    }

    json << "}";
    return json.str();
}

} // namespace data
} // namespace atlas
