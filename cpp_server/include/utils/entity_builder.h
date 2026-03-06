#ifndef NOVAFORGE_UTILS_ENTITY_BUILDER_H
#define NOVAFORGE_UTILS_ENTITY_BUILDER_H

#include "ecs/entity.h"
#include "components/core_components.h"
#include <memory>
#include <string>
#include <map>

namespace atlas {
namespace utils {

/**
 * @brief Fluent builder for creating and configuring ECS entities
 *
 * Eliminates the repetitive make_unique / set fields / addComponent
 * boilerplate that appears ~60 times across game_session_entities.cpp,
 * world_persistence.cpp, and various system files.
 *
 * Usage:
 * @code
 *   EntityBuilder(entity)
 *       .withPosition(100.0f, 0.0f, -50.0f)
 *       .withVelocity(300.0f)
 *       .withHealth(450.0f, 350.0f, 300.0f)
 *       .withShip("Fang", "Frigate", "Fang")
 *       .withFaction("Keldari")
 *       .build();
 * @endcode
 */
class EntityBuilder {
public:
    /**
     * @brief Construct a builder for the given entity
     * @param entity Non-null pointer to an already-created entity
     */
    explicit EntityBuilder(ecs::Entity* entity)
        : entity_(entity) {}

    // ---- Position ----

    EntityBuilder& withPosition(float x, float y, float z, float rotation = 0.0f) {
        auto pos = std::make_unique<components::Position>();
        pos->x = x;
        pos->y = y;
        pos->z = z;
        pos->rotation = rotation;
        entity_->addComponent(std::move(pos));
        return *this;
    }

    // ---- Velocity ----

    EntityBuilder& withVelocity(float max_speed = 0.0f) {
        auto vel = std::make_unique<components::Velocity>();
        vel->max_speed = max_speed;
        entity_->addComponent(std::move(vel));
        return *this;
    }

    // ---- Health ----

    EntityBuilder& withHealth(float shield, float armor, float hull,
                              float shield_recharge_rate = 0.0f) {
        auto hp = std::make_unique<components::Health>();
        hp->shield_hp  = hp->shield_max  = shield;
        hp->armor_hp   = hp->armor_max   = armor;
        hp->hull_hp    = hp->hull_max    = hull;
        hp->shield_recharge_rate = shield_recharge_rate;
        entity_->addComponent(std::move(hp));
        return *this;
    }

    /**
     * @brief Add Health with per-layer resistances
     */
    EntityBuilder& withHealthResists(float shield, float armor, float hull,
                                     float shield_recharge_rate,
                                     float s_em, float s_th, float s_kin, float s_exp,
                                     float a_em, float a_th, float a_kin, float a_exp,
                                     float h_em, float h_th, float h_kin, float h_exp) {
        auto hp = std::make_unique<components::Health>();
        hp->shield_hp  = hp->shield_max  = shield;
        hp->armor_hp   = hp->armor_max   = armor;
        hp->hull_hp    = hp->hull_max    = hull;
        hp->shield_recharge_rate = shield_recharge_rate;
        hp->shield_em_resist        = s_em;
        hp->shield_thermal_resist   = s_th;
        hp->shield_kinetic_resist   = s_kin;
        hp->shield_explosive_resist = s_exp;
        hp->armor_em_resist         = a_em;
        hp->armor_thermal_resist    = a_th;
        hp->armor_kinetic_resist    = a_kin;
        hp->armor_explosive_resist  = a_exp;
        hp->hull_em_resist          = h_em;
        hp->hull_thermal_resist     = h_th;
        hp->hull_kinetic_resist     = h_kin;
        hp->hull_explosive_resist   = h_exp;
        entity_->addComponent(std::move(hp));
        return *this;
    }

    /**
     * @brief Add Health with zero current HP (destroyed/wreck state)
     */
    EntityBuilder& withDestroyedHealth(float shield_max, float armor_max, float hull_max) {
        auto hp = std::make_unique<components::Health>();
        hp->shield_hp = 0.0f;  hp->shield_max = shield_max;
        hp->armor_hp  = 0.0f;  hp->armor_max  = armor_max;
        hp->hull_hp   = 0.0f;  hp->hull_max   = hull_max;
        entity_->addComponent(std::move(hp));
        return *this;
    }

    // ---- Ship ----

    EntityBuilder& withShip(const std::string& name,
                            const std::string& ship_class,
                            const std::string& ship_type,
                            const std::string& race = "") {
        auto ship = std::make_unique<components::Ship>();
        ship->ship_name  = name;
        ship->ship_class = ship_class;
        ship->ship_type  = ship_type;
        if (!race.empty()) ship->race = race;
        entity_->addComponent(std::move(ship));
        return *this;
    }

    /**
     * @brief Add Ship with full fitting stats
     */
    EntityBuilder& withShipFull(const std::string& name,
                                const std::string& ship_class,
                                const std::string& ship_type,
                                const std::string& race,
                                float cpu_max, float powergrid_max,
                                float sig_radius, float scan_res,
                                int max_targets, float max_target_range) {
        auto ship = std::make_unique<components::Ship>();
        ship->ship_name           = name;
        ship->ship_class          = ship_class;
        ship->ship_type           = ship_type;
        ship->race                = race;
        ship->cpu_max             = cpu_max;
        ship->powergrid_max       = powergrid_max;
        ship->signature_radius    = sig_radius;
        ship->scan_resolution     = scan_res;
        ship->max_locked_targets  = max_targets;
        ship->max_targeting_range = max_target_range;
        entity_->addComponent(std::move(ship));
        return *this;
    }

    // ---- Target ----

    EntityBuilder& withTarget() {
        entity_->addComponent(std::make_unique<components::Target>());
        return *this;
    }

    // ---- Player ----

    EntityBuilder& withPlayer(const std::string& player_id,
                              const std::string& character_name) {
        auto player = std::make_unique<components::Player>();
        player->player_id      = player_id;
        player->character_name = character_name;
        entity_->addComponent(std::move(player));
        return *this;
    }

    // ---- Faction ----

    EntityBuilder& withFaction(const std::string& faction_name) {
        auto fac = std::make_unique<components::Faction>();
        fac->faction_name = faction_name;
        entity_->addComponent(std::move(fac));
        return *this;
    }

    // ---- Standings ----

    EntityBuilder& withStandings(const std::map<std::string, float>& standings) {
        auto st = std::make_unique<components::Standings>();
        st->faction_standings = standings;
        entity_->addComponent(std::move(st));
        return *this;
    }

    /**
     * @brief Add default empire-neutral standings with hostile pirate factions
     */
    EntityBuilder& withDefaultPlayerStandings() {
        auto st = std::make_unique<components::Standings>();
        st->faction_standings["Veyren"]           = 0.0f;
        st->faction_standings["Aurelian"]         = 0.0f;
        st->faction_standings["Solari"]           = 0.0f;
        st->faction_standings["Keldari"]          = 0.0f;
        st->faction_standings["Venom Syndicate"]  = -5.0f;
        st->faction_standings["Iron Corsairs"]    = -5.0f;
        st->faction_standings["Crimson Order"]    = -5.0f;
        st->faction_standings["Hollow Collective"] = -5.0f;
        entity_->addComponent(std::move(st));
        return *this;
    }

    /**
     * @brief Add pirate NPC standings (hostile to all empires)
     */
    EntityBuilder& withPirateStandings() {
        auto st = std::make_unique<components::Standings>();
        st->faction_standings["Veyren"]   = -5.0f;
        st->faction_standings["Aurelian"] = -5.0f;
        st->faction_standings["Solari"]   = -5.0f;
        st->faction_standings["Keldari"]  = -5.0f;
        entity_->addComponent(std::move(st));
        return *this;
    }

    /**
     * @brief Add empire NPC standings (neutral to all empires)
     */
    EntityBuilder& withEmpireStandings() {
        auto st = std::make_unique<components::Standings>();
        st->faction_standings["Veyren"]   = 0.0f;
        st->faction_standings["Aurelian"] = 0.0f;
        st->faction_standings["Solari"]   = 0.0f;
        st->faction_standings["Keldari"]  = 0.0f;
        entity_->addComponent(std::move(st));
        return *this;
    }

    // ---- Capacitor ----

    EntityBuilder& withCapacitor(float max, float recharge_rate = 0.0f) {
        auto cap = std::make_unique<components::Capacitor>();
        cap->capacitor_max = max;
        cap->capacitor     = max;
        cap->recharge_rate = recharge_rate;
        entity_->addComponent(std::move(cap));
        return *this;
    }

    // ---- AI ----

    EntityBuilder& withAI(components::AI::Behavior behavior = components::AI::Behavior::Aggressive,
                          components::AI::State state = components::AI::State::Idle,
                          float awareness_range = 15000.0f) {
        auto ai = std::make_unique<components::AI>();
        ai->behavior = behavior;
        ai->state    = state;
        ai->awareness_range = awareness_range;
        entity_->addComponent(std::move(ai));
        return *this;
    }

    // ---- Weapon ----

    EntityBuilder& withWeapon(float damage, float optimal_range, float rate_of_fire) {
        auto weapon = std::make_unique<components::Weapon>();
        weapon->damage        = damage;
        weapon->optimal_range = optimal_range;
        weapon->rate_of_fire  = rate_of_fire;
        entity_->addComponent(std::move(weapon));
        return *this;
    }

    // ---- Generic component ----

    /**
     * @brief Add any component type (for components without dedicated helpers)
     */
    template<typename T>
    EntityBuilder& withComponent(std::unique_ptr<T> component) {
        entity_->addComponent(std::move(component));
        return *this;
    }

    /**
     * @brief Add a default-constructed component
     */
    template<typename T>
    EntityBuilder& withComponent() {
        entity_->addComponent(std::make_unique<T>());
        return *this;
    }

    // ---- Build ----

    /**
     * @brief Finalize and return the configured entity
     */
    ecs::Entity* build() { return entity_; }

private:
    ecs::Entity* entity_;
};

} // namespace utils
} // namespace atlas

#endif // NOVAFORGE_UTILS_ENTITY_BUILDER_H
