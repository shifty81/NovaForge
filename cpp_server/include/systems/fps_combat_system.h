#ifndef NOVAFORGE_SYSTEMS_FPS_COMBAT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_COMBAT_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <functional>

namespace atlas {
namespace systems {

/**
 * @brief Personal-scale FPS combat system
 *
 * Manages first-person weapon firing, reloading, and personal health/damage.
 * Uses FPS character position and look direction to determine line-of-fire.
 * Ties into the existing Astralis damage type system (EM, thermal, kinetic,
 * explosive) and works alongside SurvivalSystem (fatigue → aim penalty).
 */
class FPSCombatSystem : public ecs::System {
public:
    explicit FPSCombatSystem(ecs::World* world);
    ~FPSCombatSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FPSCombatSystem"; }

    using DeathCallback = std::function<void(const std::string& victim_id,
                                             float x, float y, float z)>;
    void setDeathCallback(DeathCallback cb) { death_callback_ = std::move(cb); }

    // --- Weapon management ---

    /** Create a personal weapon entity */
    bool createWeapon(const std::string& weapon_id,
                      const std::string& owner_id,
                      components::FPSWeapon::WeaponCategory category,
                      const std::string& damage_type,
                      float damage, float range, float fire_rate,
                      int ammo_max, float spread = 1.0f,
                      float reload_time = 2.0f);

    /** Equip a weapon (sets is_equipped flag) */
    bool equipWeapon(const std::string& weapon_id);

    /** Unequip a weapon */
    bool unequipWeapon(const std::string& weapon_id);

    /** Begin reloading the weapon */
    bool startReload(const std::string& weapon_id);

    // --- Health management ---

    /** Create personal FPS health for an entity */
    bool createHealth(const std::string& entity_id,
                      float health_max = 100.0f,
                      float shield_max = 50.0f,
                      float shield_recharge_rate = 5.0f);

    // --- Combat actions ---

    /** Fire a player's equipped weapon at a target entity */
    bool fireWeapon(const std::string& shooter_player_id,
                    const std::string& target_entity_id);

    /** Apply personal-scale damage to an entity's FPSHealth */
    bool applyDamage(const std::string& target_id, float damage,
                     const std::string& damage_type = "kinetic");

    // --- Queries ---

    float getHealth(const std::string& entity_id) const;
    float getShield(const std::string& entity_id) const;
    bool  isAlive(const std::string& entity_id) const;
    int   getAmmo(const std::string& weapon_id) const;
    bool  isReloading(const std::string& weapon_id) const;

    static std::string categoryName(int category);

private:
    DeathCallback death_callback_;

    static constexpr const char* FPS_CHAR_PREFIX = "fpschar_";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_COMBAT_SYSTEM_H
