#include "systems/rumor_questline_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

RumorQuestlineSystem::RumorQuestlineSystem(ecs::World* world)
    : System(world) {
}

void RumorQuestlineSystem::update(float /*delta_time*/) {
    auto entities = world_->getEntities<components::RumorLog>();
    for (auto* entity : entities) {
        auto* log = entity->getComponent<components::RumorLog>();
        if (!log) continue;

        for (const auto& rumor : log->rumors) {
            if (rumor.times_heard >= required_confirmations &&
                rumor.belief_strength >= belief_threshold) {
                // Check if already graduated
                auto* existing = entity->getComponent<components::RumorQuestline>();
                bool already_graduated = false;
                if (existing && existing->rumor_id == rumor.rumor_id) {
                    already_graduated = true;
                }
                // Look through all RumorQuestline components - since ECS typically
                // has one component per type, we check the single one
                if (!already_graduated && !existing) {
                    auto quest = std::make_unique<components::RumorQuestline>();
                    quest->rumor_id = rumor.rumor_id;
                    quest->questline_id = "quest_" + rumor.rumor_id;
                    quest->required_confirmations = required_confirmations;
                    quest->graduated = true;
                    quest->quest_description = "Investigate: " + rumor.text;
                    entity->addComponent(std::move(quest));
                    break; // One graduation per update tick
                }
            }
        }
    }
}

std::vector<std::string>
RumorQuestlineSystem::getGraduatedQuestlines(const std::string& entity_id) const {
    std::vector<std::string> result;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return result;

    auto* quest = entity->getComponent<components::RumorQuestline>();
    if (quest && quest->graduated) {
        result.push_back(quest->questline_id);
    }
    return result;
}

bool RumorQuestlineSystem::hasGraduatedRumor(const std::string& entity_id,
                                              const std::string& rumor_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* quest = entity->getComponent<components::RumorQuestline>();
    if (!quest) return false;
    return quest->graduated && quest->rumor_id == rumor_id;
}

} // namespace systems
} // namespace atlas
