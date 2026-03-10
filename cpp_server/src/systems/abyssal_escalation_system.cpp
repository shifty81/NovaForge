#include "systems/abyssal_escalation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AbyssalEscalationSystem::AbyssalEscalationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AbyssalEscalationSystem::updateComponent(ecs::Entity& /*entity*/,
    components::AbyssalEscalationState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    // Progression is driven by explicit API calls; nothing to tick here.
}

// ---------------------------------------------------------------------------
// Static helper: scale NPC stats for tier
// ---------------------------------------------------------------------------
static float tierScale(int tier) {
    return 1.0f + (tier - 1) * 0.5f;   // T1=1.0, T2=1.5, T3=2.0, T4=2.5, T5=3.0
}

bool AbyssalEscalationSystem::initialize(const std::string& entity_id,
    const std::string& pocket_id, int tier) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::AbyssalEscalationState>();
    comp->pocket_id = pocket_id;
    comp->tier = (std::max)(1, (std::min)(tier, 5));

    float scale = tierScale(comp->tier);

    // Wave 1 – scout pack
    components::AbyssalEscalationState::WaveConfig w1;
    w1.npc_count    = 5;
    w1.npc_base_hp  = 3000.0f * scale;
    w1.npc_base_dps = 150.0f  * scale;
    comp->waves.push_back(w1);

    // Wave 2 – reinforcements
    components::AbyssalEscalationState::WaveConfig w2;
    w2.npc_count    = 7;
    w2.npc_base_hp  = 5000.0f * scale;
    w2.npc_base_dps = 250.0f  * scale;
    comp->waves.push_back(w2);

    // Wave 3 – boss
    components::AbyssalEscalationState::WaveConfig boss;
    boss.npc_count    = 1;
    boss.npc_base_hp  = 30000.0f * scale;
    boss.npc_base_dps = 800.0f   * scale;
    comp->waves.push_back(boss);

    entity->addComponent(std::move(comp));
    return true;
}

bool AbyssalEscalationSystem::completeWave(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->active) return false;

    using Phase = components::AbyssalEscalationState::EscalationPhase;
    int wave_idx = static_cast<int>(comp->current_phase);
    if (wave_idx >= static_cast<int>(comp->waves.size())) return false;
    if (comp->waves[wave_idx].completed) return false;

    comp->waves[wave_idx].completed = true;

    // Advance phase
    if (comp->current_phase == Phase::Wave1) {
        comp->current_phase = Phase::Wave2;
    } else if (comp->current_phase == Phase::Wave2) {
        comp->current_phase = Phase::Boss;
    }
    return true;
}

bool AbyssalEscalationSystem::spawnBoss(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->active) return false;
    if (comp->current_phase !=
        components::AbyssalEscalationState::EscalationPhase::Boss) return false;
    if (comp->boss_spawned) return false;

    comp->boss_spawned = true;
    return true;
}

bool AbyssalEscalationSystem::killBoss(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->active) return false;
    if (!comp->boss_spawned || comp->boss_killed) return false;

    comp->boss_killed = true;
    // Mark boss wave complete
    int boss_idx = static_cast<int>(
        components::AbyssalEscalationState::EscalationPhase::Boss);
    if (boss_idx < static_cast<int>(comp->waves.size())) {
        comp->waves[boss_idx].completed = true;
    }
    comp->run_completed = true;
    comp->active = false;

    // Loot value scales with tier
    comp->total_loot_value = 10000000 * comp->tier;  // 10M × tier
    return true;
}

bool AbyssalEscalationSystem::applyDamage(const std::string& entity_id,
    float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->dps_received += amount;
    return true;
}

bool AbyssalEscalationSystem::recordKill(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->enemies_killed++;
    return true;
}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

components::AbyssalEscalationState::EscalationPhase
AbyssalEscalationSystem::getCurrentPhase(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_phase
                : components::AbyssalEscalationState::EscalationPhase::Wave1;
}

int AbyssalEscalationSystem::getEnemiesKilled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->enemies_killed : 0;
}

bool AbyssalEscalationSystem::isBossSpawned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->boss_spawned : false;
}

bool AbyssalEscalationSystem::isBossKilled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->boss_killed : false;
}

bool AbyssalEscalationSystem::isRunCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->run_completed : false;
}

float AbyssalEscalationSystem::getDpsReceived(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->dps_received : 0.0f;
}

int AbyssalEscalationSystem::getTotalLootValue(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_loot_value : 0;
}

} // namespace systems
} // namespace atlas
