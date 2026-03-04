// Tests for: Phase 5 Component Persistence Tests, Persistence Round-Trip for New Components, New Component Default Tests, New Component Persistence Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/economy_components.h"
#include "components/fleet_components.h"
#include "components/mission_components.h"
#include "components/narrative_components.h"
#include "components/navigation_components.h"
#include "components/npc_components.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "data/world_persistence.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Phase 5 Component Persistence Tests ====================

static void testSerializeDeserializeStation() {
    std::cout << "\n=== Serialize/Deserialize Station/Docked/Wreck ===" << std::endl;

    ecs::World world;

    auto* e1 = world.createEntity("station_entity");
    auto* station = addComp<components::Station>(e1);
    station->station_name = "Test Station";
    station->docking_range = 3000.0f;
    station->repair_cost_per_hp = 2.5f;
    station->docked_count = 3;

    auto* e2 = world.createEntity("docked_entity");
    auto* docked = addComp<components::Docked>(e2);
    docked->station_id = "station_42";

    auto* e3 = world.createEntity("wreck_entity");
    auto* wreck = addComp<components::Wreck>(e3);
    wreck->source_entity_id = "npc_frigate_7";
    wreck->lifetime_remaining = 1200.0f;
    wreck->salvaged = true;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "Station/Docked/Wreck deserialization succeeds");

    auto* s2 = world2.getEntity("station_entity");
    assertTrue(s2 != nullptr, "Station entity recreated");
    auto* station2 = s2->getComponent<components::Station>();
    assertTrue(station2 != nullptr, "Station component recreated");
    assertTrue(station2->station_name == "Test Station", "station_name preserved");
    assertTrue(approxEqual(station2->docking_range, 3000.0f), "docking_range preserved");
    assertTrue(approxEqual(station2->repair_cost_per_hp, 2.5f), "repair_cost_per_hp preserved");
    assertTrue(station2->docked_count == 3, "docked_count preserved");

    auto* d2 = world2.getEntity("docked_entity");
    assertTrue(d2 != nullptr, "Docked entity recreated");
    auto* docked2 = d2->getComponent<components::Docked>();
    assertTrue(docked2 != nullptr, "Docked component recreated");
    assertTrue(docked2->station_id == "station_42", "station_id preserved");

    auto* w2 = world2.getEntity("wreck_entity");
    assertTrue(w2 != nullptr, "Wreck entity recreated");
    auto* wreck2 = w2->getComponent<components::Wreck>();
    assertTrue(wreck2 != nullptr, "Wreck component recreated");
    assertTrue(wreck2->source_entity_id == "npc_frigate_7", "source_entity_id preserved");
    assertTrue(approxEqual(wreck2->lifetime_remaining, 1200.0f), "lifetime_remaining preserved");
    assertTrue(wreck2->salvaged == true, "salvaged preserved");
}

static void testSerializeDeserializeCaptainPersonality() {
    std::cout << "\n=== Serialize/Deserialize CaptainPersonality ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("captain_test");
    auto* cp = addComp<components::CaptainPersonality>(entity);
    cp->aggression = 0.8f;
    cp->sociability = 0.3f;
    cp->optimism = 0.9f;
    cp->professionalism = 0.1f;
    cp->loyalty = 0.7f;
    cp->paranoia = 0.6f;
    cp->ambition = 0.4f;
    cp->adaptability = 0.2f;
    cp->captain_name = "Cap. Zara";
    cp->faction = "Solari";

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "CaptainPersonality deserialization succeeds");

    auto* e2 = world2.getEntity("captain_test");
    assertTrue(e2 != nullptr, "Captain entity recreated");
    auto* cp2 = e2->getComponent<components::CaptainPersonality>();
    assertTrue(cp2 != nullptr, "CaptainPersonality component recreated");
    assertTrue(approxEqual(cp2->aggression, 0.8f), "aggression preserved");
    assertTrue(approxEqual(cp2->sociability, 0.3f), "sociability preserved");
    assertTrue(approxEqual(cp2->optimism, 0.9f), "optimism preserved");
    assertTrue(approxEqual(cp2->professionalism, 0.1f), "professionalism preserved");
    assertTrue(approxEqual(cp2->loyalty, 0.7f), "loyalty preserved");
    assertTrue(approxEqual(cp2->paranoia, 0.6f), "paranoia preserved");
    assertTrue(approxEqual(cp2->ambition, 0.4f), "ambition preserved");
    assertTrue(approxEqual(cp2->adaptability, 0.2f), "adaptability preserved");
    assertTrue(cp2->captain_name == "Cap. Zara", "captain_name preserved");
    assertTrue(cp2->faction == "Solari", "faction preserved");
}

static void testSerializeDeserializeFleetState() {
    std::cout << "\n=== Serialize/Deserialize FleetMorale/CaptainRelationship/EmotionalState ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("fleet_state_test");

    auto* morale = addComp<components::FleetMorale>(entity);
    morale->morale_score = 35.5f;
    morale->wins = 10;
    morale->losses = 3;
    morale->ships_lost = 1;
    morale->times_saved_by_player = 2;
    morale->times_player_saved = 1;
    morale->missions_together = 15;
    morale->morale_state = "Inspired";

    auto* rel = addComp<components::CaptainRelationship>(entity);
    components::CaptainRelationship::Relationship r1;
    r1.other_captain_id = "captain_alpha";
    r1.affinity = 75.0f;
    rel->relationships.push_back(r1);
    components::CaptainRelationship::Relationship r2;
    r2.other_captain_id = "captain_beta";
    r2.affinity = -30.0f;
    rel->relationships.push_back(r2);

    auto* emo = addComp<components::EmotionalState>(entity);
    emo->confidence = 80.0f;
    emo->trust_in_player = 65.0f;
    emo->fatigue = 20.0f;
    emo->hope = 70.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "FleetState deserialization succeeds");

    auto* e2 = world2.getEntity("fleet_state_test");
    assertTrue(e2 != nullptr, "Fleet state entity recreated");

    auto* morale2 = e2->getComponent<components::FleetMorale>();
    assertTrue(morale2 != nullptr, "FleetMorale component recreated");
    assertTrue(approxEqual(morale2->morale_score, 35.5f), "morale_score preserved");
    assertTrue(morale2->wins == 10, "wins preserved");
    assertTrue(morale2->losses == 3, "losses preserved");
    assertTrue(morale2->ships_lost == 1, "ships_lost preserved");
    assertTrue(morale2->times_saved_by_player == 2, "times_saved_by_player preserved");
    assertTrue(morale2->times_player_saved == 1, "times_player_saved preserved");
    assertTrue(morale2->missions_together == 15, "missions_together preserved");
    assertTrue(morale2->morale_state == "Inspired", "morale_state preserved");

    auto* rel2 = e2->getComponent<components::CaptainRelationship>();
    assertTrue(rel2 != nullptr, "CaptainRelationship component recreated");
    assertTrue(rel2->relationships.size() == 2, "relationship count preserved");
    assertTrue(rel2->relationships[0].other_captain_id == "captain_alpha", "relationship[0] id preserved");
    assertTrue(approxEqual(rel2->relationships[0].affinity, 75.0f), "relationship[0] affinity preserved");
    assertTrue(rel2->relationships[1].other_captain_id == "captain_beta", "relationship[1] id preserved");
    assertTrue(approxEqual(rel2->relationships[1].affinity, -30.0f), "relationship[1] affinity preserved");

    auto* emo2 = e2->getComponent<components::EmotionalState>();
    assertTrue(emo2 != nullptr, "EmotionalState component recreated");
    assertTrue(approxEqual(emo2->confidence, 80.0f), "confidence preserved");
    assertTrue(approxEqual(emo2->trust_in_player, 65.0f), "trust_in_player preserved");
    assertTrue(approxEqual(emo2->fatigue, 20.0f), "fatigue preserved");
    assertTrue(approxEqual(emo2->hope, 70.0f), "hope preserved");
}

static void testSerializeDeserializeCaptainMemory() {
    std::cout << "\n=== Serialize/Deserialize CaptainMemory ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("memory_test");
    auto* mem = addComp<components::CaptainMemory>(entity);
    mem->max_memories = 25;

    components::CaptainMemory::MemoryEntry m1;
    m1.event_type = "combat_win";
    m1.context = "Defeated pirate frigate";
    m1.timestamp = 1000.0f;
    m1.emotional_weight = 0.8f;
    mem->memories.push_back(m1);

    components::CaptainMemory::MemoryEntry m2;
    m2.event_type = "ship_lost";
    m2.context = "Wingman destroyed";
    m2.timestamp = 2000.0f;
    m2.emotional_weight = -0.9f;
    mem->memories.push_back(m2);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "CaptainMemory deserialization succeeds");

    auto* e2 = world2.getEntity("memory_test");
    assertTrue(e2 != nullptr, "Memory entity recreated");
    auto* mem2 = e2->getComponent<components::CaptainMemory>();
    assertTrue(mem2 != nullptr, "CaptainMemory component recreated");
    assertTrue(mem2->max_memories == 25, "max_memories preserved");
    assertTrue(mem2->memories.size() == 2, "memory count preserved");
    assertTrue(mem2->memories[0].event_type == "combat_win", "memory[0] event_type preserved");
    assertTrue(mem2->memories[0].context == "Defeated pirate frigate", "memory[0] context preserved");
    assertTrue(approxEqual(mem2->memories[0].timestamp, 1000.0f), "memory[0] timestamp preserved");
    assertTrue(approxEqual(mem2->memories[0].emotional_weight, 0.8f), "memory[0] emotional_weight preserved");
    assertTrue(mem2->memories[1].event_type == "ship_lost", "memory[1] event_type preserved");
    assertTrue(mem2->memories[1].context == "Wingman destroyed", "memory[1] context preserved");
    assertTrue(approxEqual(mem2->memories[1].timestamp, 2000.0f), "memory[1] timestamp preserved");
    assertTrue(approxEqual(mem2->memories[1].emotional_weight, -0.9f), "memory[1] emotional_weight preserved");
}

static void testSerializeDeserializeFleetFormation() {
    std::cout << "\n=== Serialize/Deserialize FleetFormation ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("formation_test");
    auto* ff = addComp<components::FleetFormation>(entity);
    ff->formation = components::FleetFormation::FormationType::Wedge;
    ff->slot_index = 2;
    ff->offset_x = 150.0f;
    ff->offset_y = -50.0f;
    ff->offset_z = 0.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "FleetFormation deserialization succeeds");

    auto* e2 = world2.getEntity("formation_test");
    assertTrue(e2 != nullptr, "Formation entity recreated");
    auto* ff2 = e2->getComponent<components::FleetFormation>();
    assertTrue(ff2 != nullptr, "FleetFormation component recreated");
    assertTrue(static_cast<int>(ff2->formation) == 3, "formation type preserved (Wedge=3)");
    assertTrue(ff2->slot_index == 2, "slot_index preserved");
    assertTrue(approxEqual(ff2->offset_x, 150.0f), "offset_x preserved");
    assertTrue(approxEqual(ff2->offset_y, -50.0f), "offset_y preserved");
    assertTrue(approxEqual(ff2->offset_z, 0.0f), "offset_z preserved");
}

static void testSerializeDeserializeFleetCargoAndRumors() {
    std::cout << "\n=== Serialize/Deserialize FleetCargoPool/RumorLog ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("cargo_rumor_test");

    auto* cargo = addComp<components::FleetCargoPool>(entity);
    cargo->total_capacity = 50000;
    cargo->used_capacity = 12000;
    cargo->pooled_items["Stellium"] = 5000;
    cargo->pooled_items["Vanthium"] = 2000;
    cargo->contributor_ship_ids.push_back("ship_1");
    cargo->contributor_ship_ids.push_back("ship_2");

    auto* rumor = addComp<components::RumorLog>(entity);
    components::RumorLog::Rumor r1;
    r1.rumor_id = "rumor_01";
    r1.text = "Pirates near gate";
    r1.belief_strength = 0.7f;
    r1.personally_witnessed = true;
    r1.times_heard = 3;
    rumor->rumors.push_back(r1);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "FleetCargoPool/RumorLog deserialization succeeds");

    auto* e2 = world2.getEntity("cargo_rumor_test");
    assertTrue(e2 != nullptr, "Cargo/Rumor entity recreated");

    auto* cargo2 = e2->getComponent<components::FleetCargoPool>();
    assertTrue(cargo2 != nullptr, "FleetCargoPool component recreated");
    assertTrue(cargo2->total_capacity == 50000, "total_capacity preserved");
    assertTrue(cargo2->used_capacity == 12000, "used_capacity preserved");
    assertTrue(cargo2->pooled_items.size() == 2, "pooled_items count preserved");
    assertTrue(cargo2->pooled_items["Stellium"] == 5000, "Stellium quantity preserved");
    assertTrue(cargo2->pooled_items["Vanthium"] == 2000, "Vanthium quantity preserved");
    assertTrue(cargo2->contributor_ship_ids.size() == 2, "contributor count preserved");
    assertTrue(cargo2->contributor_ship_ids[0] == "ship_1", "contributor[0] preserved");
    assertTrue(cargo2->contributor_ship_ids[1] == "ship_2", "contributor[1] preserved");

    auto* rumor2 = e2->getComponent<components::RumorLog>();
    assertTrue(rumor2 != nullptr, "RumorLog component recreated");
    assertTrue(rumor2->rumors.size() == 1, "rumor count preserved");
    assertTrue(rumor2->rumors[0].rumor_id == "rumor_01", "rumor_id preserved");
    assertTrue(rumor2->rumors[0].text == "Pirates near gate", "rumor text preserved");
    assertTrue(approxEqual(rumor2->rumors[0].belief_strength, 0.7f), "belief_strength preserved");
    assertTrue(rumor2->rumors[0].personally_witnessed == true, "personally_witnessed preserved");
    assertTrue(rumor2->rumors[0].times_heard == 3, "times_heard preserved");
}

static void testSerializeDeserializeEconomyComponents() {
    std::cout << "\n=== Serialize/Deserialize MineralDeposit/SystemResources/MarketHub ===" << std::endl;

    ecs::World world;

    auto* e1 = world.createEntity("deposit_entity");
    auto* deposit = addComp<components::MineralDeposit>(e1);
    deposit->mineral_type = "Galvite";
    deposit->quantity_remaining = 7500.0f;
    deposit->max_quantity = 10000.0f;
    deposit->yield_rate = 1.2f;
    deposit->volume_per_unit = 0.15f;

    auto* e2 = world.createEntity("sysres_entity");
    auto* sysres = addComp<components::SystemResources>(e2);
    components::SystemResources::ResourceEntry re1;
    re1.mineral_type = "Stellium";
    re1.total_quantity = 50000.0f;
    re1.remaining_quantity = 35000.0f;
    sysres->resources.push_back(re1);
    components::SystemResources::ResourceEntry re2;
    re2.mineral_type = "Vanthium";
    re2.total_quantity = 20000.0f;
    re2.remaining_quantity = 18000.0f;
    sysres->resources.push_back(re2);

    auto* e3 = world.createEntity("market_entity");
    auto* market = addComp<components::MarketHub>(e3);
    market->station_id = "hub_jita";
    market->broker_fee_rate = 0.03;
    market->sales_tax_rate = 0.05;
    components::MarketHub::Order order;
    order.order_id = "order_001";
    order.item_id = "trit";
    order.item_name = "Stellium";
    order.owner_id = "player_1";
    order.is_buy_order = true;
    order.price_per_unit = 5.5;
    order.quantity = 1000;
    order.quantity_remaining = 800;
    order.duration_remaining = 86400.0f;
    order.fulfilled = false;
    market->orders.push_back(order);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "Economy components deserialization succeeds");

    auto* de = world2.getEntity("deposit_entity");
    assertTrue(de != nullptr, "Deposit entity recreated");
    auto* deposit2 = de->getComponent<components::MineralDeposit>();
    assertTrue(deposit2 != nullptr, "MineralDeposit component recreated");
    assertTrue(deposit2->mineral_type == "Galvite", "mineral_type preserved");
    assertTrue(approxEqual(deposit2->quantity_remaining, 7500.0f), "quantity_remaining preserved");
    assertTrue(approxEqual(deposit2->max_quantity, 10000.0f), "max_quantity preserved");
    assertTrue(approxEqual(deposit2->yield_rate, 1.2f), "yield_rate preserved");
    assertTrue(approxEqual(deposit2->volume_per_unit, 0.15f), "volume_per_unit preserved");

    auto* sr = world2.getEntity("sysres_entity");
    assertTrue(sr != nullptr, "SystemResources entity recreated");
    auto* sysres2 = sr->getComponent<components::SystemResources>();
    assertTrue(sysres2 != nullptr, "SystemResources component recreated");
    assertTrue(sysres2->resources.size() == 2, "resource count preserved");
    assertTrue(sysres2->resources[0].mineral_type == "Stellium", "resource[0] mineral_type preserved");
    assertTrue(approxEqual(sysres2->resources[0].total_quantity, 50000.0f), "resource[0] total_quantity preserved");
    assertTrue(approxEqual(sysres2->resources[0].remaining_quantity, 35000.0f), "resource[0] remaining_quantity preserved");
    assertTrue(sysres2->resources[1].mineral_type == "Vanthium", "resource[1] mineral_type preserved");
    assertTrue(approxEqual(sysres2->resources[1].total_quantity, 20000.0f), "resource[1] total_quantity preserved");
    assertTrue(approxEqual(sysres2->resources[1].remaining_quantity, 18000.0f), "resource[1] remaining_quantity preserved");

    auto* me = world2.getEntity("market_entity");
    assertTrue(me != nullptr, "MarketHub entity recreated");
    auto* market2 = me->getComponent<components::MarketHub>();
    assertTrue(market2 != nullptr, "MarketHub component recreated");
    assertTrue(market2->station_id == "hub_jita", "market station_id preserved");
    assertTrue(approxEqual(static_cast<float>(market2->broker_fee_rate), 0.03f), "broker_fee_rate preserved");
    assertTrue(approxEqual(static_cast<float>(market2->sales_tax_rate), 0.05f), "sales_tax_rate preserved");
    assertTrue(market2->orders.size() == 1, "order count preserved");
    assertTrue(market2->orders[0].order_id == "order_001", "order_id preserved");
    assertTrue(market2->orders[0].item_id == "trit", "order item_id preserved");
    assertTrue(market2->orders[0].item_name == "Stellium", "order item_name preserved");
    assertTrue(market2->orders[0].owner_id == "player_1", "order owner_id preserved");
    assertTrue(market2->orders[0].is_buy_order == true, "is_buy_order preserved");
    assertTrue(approxEqual(static_cast<float>(market2->orders[0].price_per_unit), 5.5f), "price_per_unit preserved");
    assertTrue(market2->orders[0].quantity == 1000, "order quantity preserved");
    assertTrue(market2->orders[0].quantity_remaining == 800, "order quantity_remaining preserved");
    assertTrue(approxEqual(market2->orders[0].duration_remaining, 86400.0f), "order duration_remaining preserved");
    assertTrue(market2->orders[0].fulfilled == false, "order fulfilled preserved");
}


// ==================== Persistence Round-Trip for New Components ====================

static void testPersistenceAnomalyVisualCue() {
    std::cout << "\n=== Persistence: AnomalyVisualCue Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("anom_vis1");
    auto* cue = addComp<components::AnomalyVisualCue>(e);
    cue->anomaly_id = "anom_42";
    cue->cue_type = components::AnomalyVisualCue::CueType::GravityLens;
    cue->intensity = 0.8f;
    cue->radius = 750.0f;
    cue->pulse_frequency = 1.5f;
    cue->r = 0.2f; cue->g = 0.5f; cue->b = 0.9f;
    cue->distortion_strength = 0.7f;
    cue->active = true;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with AnomalyVisualCue");

    auto* e2 = world2.getEntity("anom_vis1");
    assertTrue(e2 != nullptr, "Entity found after deserialization");
    auto* cue2 = e2->getComponent<components::AnomalyVisualCue>();
    assertTrue(cue2 != nullptr, "AnomalyVisualCue component present");
    assertTrue(cue2->anomaly_id == "anom_42", "anomaly_id preserved");
    assertTrue(cue2->cue_type == components::AnomalyVisualCue::CueType::GravityLens, "cue_type preserved");
    assertTrue(approxEqual(cue2->intensity, 0.8f), "intensity preserved");
    assertTrue(approxEqual(cue2->radius, 750.0f), "radius preserved");
    assertTrue(approxEqual(cue2->pulse_frequency, 1.5f), "pulse_frequency preserved");
    assertTrue(approxEqual(cue2->r, 0.2f), "r preserved");
    assertTrue(approxEqual(cue2->g, 0.5f), "g preserved");
    assertTrue(approxEqual(cue2->b, 0.9f), "b preserved");
    assertTrue(approxEqual(cue2->distortion_strength, 0.7f), "distortion_strength preserved");
    assertTrue(cue2->active, "active preserved");
}

static void testPersistenceLODPriority() {
    std::cout << "\n=== Persistence: LODPriority Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("lod_ent1");
    auto* lod = addComp<components::LODPriority>(e);
    lod->priority = 2.5f;
    lod->force_visible = true;
    lod->impostor_distance = 800.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with LODPriority");

    auto* e2 = world2.getEntity("lod_ent1");
    assertTrue(e2 != nullptr, "Entity found after deserialization");
    auto* lod2 = e2->getComponent<components::LODPriority>();
    assertTrue(lod2 != nullptr, "LODPriority component present");
    assertTrue(approxEqual(lod2->priority, 2.5f), "priority preserved");
    assertTrue(lod2->force_visible, "force_visible preserved");
    assertTrue(approxEqual(lod2->impostor_distance, 800.0f), "impostor_distance preserved");
}


// ==================== New Component Default Tests ====================

static void testWarpProfileDefaults() {
    std::cout << "\n=== WarpProfile Defaults ===" << std::endl;
    components::WarpProfile wp;
    assertTrue(approxEqual(wp.warp_speed, 3.0f), "WarpProfile default warp_speed is 3.0");
    assertTrue(approxEqual(wp.mass_norm, 0.0f), "WarpProfile default mass_norm is 0.0");
    assertTrue(approxEqual(wp.intensity, 0.0f), "WarpProfile default intensity is 0.0");
    assertTrue(approxEqual(wp.comfort_scale, 1.0f), "WarpProfile default comfort_scale is 1.0");
}

static void testWarpVisualDefaults() {
    std::cout << "\n=== WarpVisual Defaults ===" << std::endl;
    components::WarpVisual wv;
    assertTrue(approxEqual(wv.distortion_strength, 0.0f), "WarpVisual default distortion_strength is 0.0");
    assertTrue(approxEqual(wv.tunnel_noise_scale, 1.0f), "WarpVisual default tunnel_noise_scale is 1.0");
    assertTrue(approxEqual(wv.vignette_amount, 0.0f), "WarpVisual default vignette_amount is 0.0");
    assertTrue(approxEqual(wv.bloom_strength, 0.0f), "WarpVisual default bloom_strength is 0.0");
    assertTrue(approxEqual(wv.starfield_speed, 1.0f), "WarpVisual default starfield_speed is 1.0");
}

static void testWarpEventDefaults() {
    std::cout << "\n=== WarpEvent Defaults ===" << std::endl;
    components::WarpEvent we;
    assertTrue(we.current_event.empty(), "WarpEvent default current_event is empty");
    assertTrue(approxEqual(we.event_timer, 0.0f), "WarpEvent default event_timer is 0.0");
    assertTrue(we.severity == 0, "WarpEvent default severity is 0");
}

static void testTacticalProjectionDefaults() {
    std::cout << "\n=== TacticalProjection Defaults ===" << std::endl;
    components::TacticalProjection tp;
    assertTrue(approxEqual(tp.projected_x, 0.0f), "TacticalProjection default projected_x is 0.0");
    assertTrue(approxEqual(tp.projected_y, 0.0f), "TacticalProjection default projected_y is 0.0");
    assertTrue(approxEqual(tp.vertical_offset, 0.0f), "TacticalProjection default vertical_offset is 0.0");
    assertTrue(tp.visible, "TacticalProjection default visible is true");
}

static void testPlayerPresenceDefaults() {
    std::cout << "\n=== PlayerPresence Defaults ===" << std::endl;
    components::PlayerPresence pp;
    assertTrue(approxEqual(pp.time_since_last_command, 0.0f), "PlayerPresence default time_since_last_command is 0.0");
    assertTrue(approxEqual(pp.time_since_last_speech, 0.0f), "PlayerPresence default time_since_last_speech is 0.0");
}

static void testFactionCultureDefaults() {
    std::cout << "\n=== FactionCulture Defaults ===" << std::endl;
    components::FactionCulture fc;
    assertTrue(fc.faction.empty(), "FactionCulture default faction is empty");
    assertTrue(approxEqual(fc.chatter_frequency_mod, 1.0f), "FactionCulture default chatter_frequency_mod is 1.0");
    assertTrue(approxEqual(fc.formation_tightness_mod, 1.0f), "FactionCulture default formation_tightness_mod is 1.0");
    assertTrue(approxEqual(fc.morale_sensitivity, 1.0f), "FactionCulture default morale_sensitivity is 1.0");
    assertTrue(approxEqual(fc.risk_tolerance, 0.5f), "FactionCulture default risk_tolerance is 0.5");
}


// ==================== New Component Persistence Tests ====================

static void testPersistenceWarpProfile() {
    std::cout << "\n=== Persistence: WarpProfile Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("warp_prof1");
    auto* wp = addComp<components::WarpProfile>(e);
    wp->warp_speed = 5.5f;
    wp->mass_norm = 0.8f;
    wp->intensity = 0.6f;
    wp->comfort_scale = 0.7f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with WarpProfile");

    auto* e2 = world2.getEntity("warp_prof1");
    assertTrue(e2 != nullptr, "WarpProfile entity found");
    auto* wp2 = e2->getComponent<components::WarpProfile>();
    assertTrue(wp2 != nullptr, "WarpProfile component present");
    assertTrue(approxEqual(wp2->warp_speed, 5.5f), "warp_speed preserved");
    assertTrue(approxEqual(wp2->mass_norm, 0.8f), "mass_norm preserved");
    assertTrue(approxEqual(wp2->intensity, 0.6f), "intensity preserved");
    assertTrue(approxEqual(wp2->comfort_scale, 0.7f), "comfort_scale preserved");
}

static void testPersistenceWarpVisual() {
    std::cout << "\n=== Persistence: WarpVisual Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("warp_vis1");
    auto* wv = addComp<components::WarpVisual>(e);
    wv->distortion_strength = 0.9f;
    wv->tunnel_noise_scale = 2.0f;
    wv->vignette_amount = 0.4f;
    wv->bloom_strength = 0.5f;
    wv->starfield_speed = 3.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with WarpVisual");

    auto* e2 = world2.getEntity("warp_vis1");
    assertTrue(e2 != nullptr, "WarpVisual entity found");
    auto* wv2 = e2->getComponent<components::WarpVisual>();
    assertTrue(wv2 != nullptr, "WarpVisual component present");
    assertTrue(approxEqual(wv2->distortion_strength, 0.9f), "distortion_strength preserved");
    assertTrue(approxEqual(wv2->tunnel_noise_scale, 2.0f), "tunnel_noise_scale preserved");
    assertTrue(approxEqual(wv2->vignette_amount, 0.4f), "vignette_amount preserved");
    assertTrue(approxEqual(wv2->bloom_strength, 0.5f), "bloom_strength preserved");
    assertTrue(approxEqual(wv2->starfield_speed, 3.0f), "starfield_speed preserved");
}

static void testPersistenceWarpEvent() {
    std::cout << "\n=== Persistence: WarpEvent Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("warp_evt1");
    auto* we = addComp<components::WarpEvent>(e);
    we->current_event = "tunnel_eddy";
    we->event_timer = 4.5f;
    we->severity = 2;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with WarpEvent");

    auto* e2 = world2.getEntity("warp_evt1");
    assertTrue(e2 != nullptr, "WarpEvent entity found");
    auto* we2 = e2->getComponent<components::WarpEvent>();
    assertTrue(we2 != nullptr, "WarpEvent component present");
    assertTrue(we2->current_event == "tunnel_eddy", "current_event preserved");
    assertTrue(approxEqual(we2->event_timer, 4.5f), "event_timer preserved");
    assertTrue(we2->severity == 2, "severity preserved");
}

static void testPersistenceTacticalProjection() {
    std::cout << "\n=== Persistence: TacticalProjection Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("tact_proj1");
    auto* tp = addComp<components::TacticalProjection>(e);
    tp->projected_x = 150.0f;
    tp->projected_y = 250.0f;
    tp->vertical_offset = -10.5f;
    tp->visible = false;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with TacticalProjection");

    auto* e2 = world2.getEntity("tact_proj1");
    assertTrue(e2 != nullptr, "TacticalProjection entity found");
    auto* tp2 = e2->getComponent<components::TacticalProjection>();
    assertTrue(tp2 != nullptr, "TacticalProjection component present");
    assertTrue(approxEqual(tp2->projected_x, 150.0f), "projected_x preserved");
    assertTrue(approxEqual(tp2->projected_y, 250.0f), "projected_y preserved");
    assertTrue(approxEqual(tp2->vertical_offset, -10.5f), "vertical_offset preserved");
    assertTrue(!tp2->visible, "visible preserved as false");
}

static void testPersistencePlayerPresence() {
    std::cout << "\n=== Persistence: PlayerPresence Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("player_pres1");
    auto* pp = addComp<components::PlayerPresence>(e);
    pp->time_since_last_command = 45.0f;
    pp->time_since_last_speech = 120.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with PlayerPresence");

    auto* e2 = world2.getEntity("player_pres1");
    assertTrue(e2 != nullptr, "PlayerPresence entity found");
    auto* pp2 = e2->getComponent<components::PlayerPresence>();
    assertTrue(pp2 != nullptr, "PlayerPresence component present");
    assertTrue(approxEqual(pp2->time_since_last_command, 45.0f), "time_since_last_command preserved");
    assertTrue(approxEqual(pp2->time_since_last_speech, 120.0f), "time_since_last_speech preserved");
}

static void testPersistenceFactionCulture() {
    std::cout << "\n=== Persistence: FactionCulture Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("fc_solari1");
    auto* fc = addComp<components::FactionCulture>(e);
    fc->faction = "Solari";
    fc->chatter_frequency_mod = 1.2f;
    fc->formation_tightness_mod = 0.9f;
    fc->morale_sensitivity = 1.3f;
    fc->risk_tolerance = 0.3f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with FactionCulture");

    auto* e2 = world2.getEntity("fc_solari1");
    assertTrue(e2 != nullptr, "FactionCulture entity found");
    auto* fc2 = e2->getComponent<components::FactionCulture>();
    assertTrue(fc2 != nullptr, "FactionCulture component present");
    assertTrue(fc2->faction == "Solari", "faction preserved");
    assertTrue(approxEqual(fc2->chatter_frequency_mod, 1.2f), "chatter_frequency_mod preserved");
    assertTrue(approxEqual(fc2->formation_tightness_mod, 0.9f), "formation_tightness_mod preserved");
    assertTrue(approxEqual(fc2->morale_sensitivity, 1.3f), "morale_sensitivity preserved");
    assertTrue(approxEqual(fc2->risk_tolerance, 0.3f), "risk_tolerance preserved");
}


void run_persistence_tests() {
    testSerializeDeserializeStation();
    testSerializeDeserializeCaptainPersonality();
    testSerializeDeserializeFleetState();
    testSerializeDeserializeCaptainMemory();
    testSerializeDeserializeFleetFormation();
    testSerializeDeserializeFleetCargoAndRumors();
    testSerializeDeserializeEconomyComponents();
    testPersistenceAnomalyVisualCue();
    testPersistenceLODPriority();
    testWarpProfileDefaults();
    testWarpVisualDefaults();
    testWarpEventDefaults();
    testTacticalProjectionDefaults();
    testPlayerPresenceDefaults();
    testFactionCultureDefaults();
    testPersistenceWarpProfile();
    testPersistenceWarpVisual();
    testPersistenceWarpEvent();
    testPersistenceTacticalProjection();
    testPersistencePlayerPresence();
    testPersistenceFactionCulture();
}
