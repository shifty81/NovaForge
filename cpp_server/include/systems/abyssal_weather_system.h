#ifndef NOVAFORGE_SYSTEMS_ABYSSAL_WEATHER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ABYSSAL_WEATHER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Abyssal Deadspace weather environment system
 *
 * Each Abyssal pocket has a weather type that applies global
 * modifiers to all ships inside:
 *   - Electrical  : boosts turret optimal range; debuffs capacitor recharge
 *   - Dark Matter  : reduces drone speed and visibility
 *   - Exotic Plasma: boosts missile velocity; debuffs armor HP
 *   - Gamma        : boosts shield HP; debuffs hull HP
 *   - Firestorm    : boosts EW strength; debuffs propulsion
 *
 * Intensity scales linearly with filament tier (1.0 at T1, 5.0 at T5).
 */
class AbyssalWeatherSystem
    : public ecs::SingleComponentSystem<components::AbyssalWeatherState> {
public:
    explicit AbyssalWeatherSystem(ecs::World* world);
    ~AbyssalWeatherSystem() override = default;

    std::string getName() const override { return "AbyssalWeatherSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& pocket_id = "");
    bool setWeather(const std::string& entity_id,
                    components::AbyssalWeatherState::WeatherType type,
                    int tier = 1);

    components::AbyssalWeatherState::WeatherType
        getWeather(const std::string& entity_id) const;
    float getIntensity(const std::string& entity_id) const;
    components::AbyssalWeatherState::WeatherEffect
        getEffect(const std::string& entity_id) const;

    float getTurretOptimalModifier(const std::string& entity_id) const;
    float getMissileVelocityModifier(const std::string& entity_id) const;
    float getDroneSpeedModifier(const std::string& entity_id) const;
    float getShieldHpModifier(const std::string& entity_id) const;
    float getArmorHpModifier(const std::string& entity_id) const;
    float getCapacitorRechargeModifier(const std::string& entity_id) const;
    float getPropulsionModifier(const std::string& entity_id) const;
    float getEwStrengthModifier(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AbyssalWeatherState& comp,
                         float delta_time) override;

private:
    static components::AbyssalWeatherState::WeatherEffect
        buildEffect(components::AbyssalWeatherState::WeatherType type, float intensity);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ABYSSAL_WEATHER_SYSTEM_H
