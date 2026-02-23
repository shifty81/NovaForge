#ifndef EVE_SYSTEMS_RUMOR_QUESTLINE_SYSTEM_H
#define EVE_SYSTEMS_RUMOR_QUESTLINE_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Evaluates rumors for questline graduation
 *
 * When a rumor has been heard enough times and has sufficient belief,
 * it graduates into a full questline.
 */
class RumorQuestlineSystem : public ecs::System {
public:
    explicit RumorQuestlineSystem(ecs::World* world);
    ~RumorQuestlineSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "RumorQuestlineSystem"; }

    // --- Query API ---
    std::vector<std::string> getGraduatedQuestlines(const std::string& entity_id) const;
    bool hasGraduatedRumor(const std::string& entity_id, const std::string& rumor_id) const;

    // --- Configuration ---
    int required_confirmations = 3;
    float belief_threshold = 0.7f;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_RUMOR_QUESTLINE_SYSTEM_H
