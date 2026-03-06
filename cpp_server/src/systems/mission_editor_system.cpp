#include "systems/mission_editor_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

MissionEditorSystem::MissionEditorSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void MissionEditorSystem::updateComponent(ecs::Entity& /*entity*/, components::MissionEditor& ed, float /*delta_time*/) {
    if (!ed.active) return;
    // Editor doesn't need tick-based updates — all operations are explicit
}

bool MissionEditorSystem::createEditor(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MissionEditor>();
    entity->addComponent(std::move(comp));
    return true;
}

bool MissionEditorSystem::setMissionName(const std::string& entity_id, const std::string& name) {
    auto* ed = getComponentFor(entity_id);
    if (!ed) return false;
    ed->mission_name = name;
    return true;
}

bool MissionEditorSystem::setMissionLevel(const std::string& entity_id, int level) {
    auto* ed = getComponentFor(entity_id);
    if (!ed) return false;
    ed->mission_level = std::max(1, std::min(level, 5));
    return true;
}

bool MissionEditorSystem::setMissionType(const std::string& entity_id, int type) {
    auto* ed = getComponentFor(entity_id);
    if (!ed) return false;
    if (type < 0 || type > 4) return false;
    ed->mission_type = type;
    return true;
}

int MissionEditorSystem::addObjective(const std::string& entity_id,
                                       const std::string& description, int obj_type) {
    auto* ed = getComponentFor(entity_id);
    if (!ed) return -1;
    if (description.empty()) return -1;

    components::MissionEditor::Objective obj;
    obj.id = ed->next_objective_id++;
    obj.description = description;
    obj.type = std::max(0, std::min(obj_type, 6));
    ed->objectives.push_back(obj);
    return obj.id;
}

bool MissionEditorSystem::removeObjective(const std::string& entity_id, int objective_id) {
    auto* ed = getComponentFor(entity_id);
    if (!ed) return false;

    auto it = std::find_if(ed->objectives.begin(), ed->objectives.end(),
        [objective_id](const components::MissionEditor::Objective& o) { return o.id == objective_id; });
    if (it == ed->objectives.end()) return false;
    ed->objectives.erase(it);
    return true;
}

bool MissionEditorSystem::setReward(const std::string& entity_id, float credits, float standing) {
    auto* ed = getComponentFor(entity_id);
    if (!ed) return false;
    ed->reward_credits = std::max(0.0f, credits);
    ed->reward_standing = standing;
    return true;
}

bool MissionEditorSystem::validate(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ed = entity->getComponent<components::MissionEditor>();
    if (!ed) return false;

    ed->validation_error.clear();

    if (ed->mission_name.empty()) {
        ed->validation_error = "Mission name is required";
        return false;
    }
    if (ed->objectives.empty()) {
        ed->validation_error = "At least one objective is required";
        return false;
    }
    if (ed->reward_credits <= 0.0f) {
        ed->validation_error = "Reward credits must be positive";
        return false;
    }
    return true;
}

bool MissionEditorSystem::publish(const std::string& entity_id) {
    auto* ed = getComponentFor(entity_id);
    if (!ed) return false;

    if (!validate(entity_id)) return false;
    ed->published_count++;
    return true;
}

int MissionEditorSystem::getObjectiveCount(const std::string& entity_id) const {
    const auto* ed = getComponentFor(entity_id);
    if (!ed) return 0;
    return static_cast<int>(ed->objectives.size());
}

int MissionEditorSystem::getPublishedCount(const std::string& entity_id) const {
    const auto* ed = getComponentFor(entity_id);
    if (!ed) return 0;
    return ed->published_count;
}

std::string MissionEditorSystem::getValidationError(const std::string& entity_id) const {
    const auto* ed = getComponentFor(entity_id);
    if (!ed) return "";
    return ed->validation_error;
}

} // namespace systems
} // namespace atlas
