#ifndef NOVAFORGE_SYSTEMS_WARP_AUTO_COMFORT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WARP_AUTO_COMFORT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * WarpAutoComfortSystem — automatically reduces warp visual intensity
 * when frame rate drops or ultrawide displays are detected.
 *
 * Reads WarpAutoComfort (FPS data, ultrawide flag).
 * Writes WarpAccessibility (motion/blur/bass scale adjustments).
 *
 * Rules:
 *   - If current_fps < target_fps * 0.8: ramp comfort_reduction up
 *   - If current_fps >= target_fps * 0.95: ramp comfort_reduction down
 *   - Apply comfort_reduction as a multiplier on motion_intensity and blur_intensity
 *   - If ultrawide_detected: clamp radial distortion (via blur_intensity)
 *   - Oscillation frequency reduced proportionally to comfort_reduction
 */
class WarpAutoComfortSystem : public ecs::SingleComponentSystem<components::WarpAutoComfort> {
public:
    explicit WarpAutoComfortSystem(ecs::World* world);
    ~WarpAutoComfortSystem() override = default;

    std::string getName() const override { return "WarpAutoComfortSystem"; }

    /**
     * Compute the comfort reduction factor from FPS data.
     * Static for independent testing.
     * @param current_fps     Measured frame rate
     * @param target_fps      Desired frame rate
     * @param current_reduction  Current comfort_reduction value
     * @param delta_time      Frame time
     * @return Updated comfort_reduction (0–1)
     */
    static float computeComfortReduction(float current_fps, float target_fps,
                                         float current_reduction, float delta_time);

    /**
     * Apply comfort reduction + ultrawide clamp to accessibility settings.
     * @param comfort_reduction   Current reduction factor (0–1)
     * @param ultrawide           Whether ultrawide display is detected
     * @param max_distortion_uw   Max distortion cap for ultrawide
     * @param[in,out] motion      Motion intensity (modified in-place)
     * @param[in,out] blur        Blur intensity (modified in-place)
     */
    static void applyComfort(float comfort_reduction, bool ultrawide,
                             float max_distortion_uw,
                             float& motion, float& blur);

protected:
    void updateComponent(ecs::Entity& entity, components::WarpAutoComfort& comfort, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WARP_AUTO_COMFORT_SYSTEM_H
