#include "systems/damage_feedback_hud_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ui_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

DamageFeedbackHudSystem::DamageFeedbackHudSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DamageFeedbackHudSystem::updateComponent(ecs::Entity& /*entity*/,
    components::DamageFeedbackHud& fb, float delta_time) {
    if (!fb.active) return;

    // Decay each feedback layer
    auto decayLayer = [&](components::DamageFeedbackHud::FeedbackLayer& layer) {
        if (!layer.active) return;
        layer.duration += delta_time;
        layer.intensity -= layer.decay_rate * delta_time;
        if (layer.intensity <= 0.0f) {
            layer.intensity = 0.0f;
            layer.active = false;
            layer.duration = 0.0f;
        }
    };

    decayLayer(fb.shield_feedback);
    decayLayer(fb.armor_feedback);
    decayLayer(fb.hull_feedback);

    // Decay screen shake
    if (fb.screen_shake_intensity > 0.0f) {
        fb.screen_shake_intensity -= fb.screen_shake_decay * delta_time;
        if (fb.screen_shake_intensity < 0.0f)
            fb.screen_shake_intensity = 0.0f;
    }
}

bool DamageFeedbackHudSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DamageFeedbackHud>();
    entity->addComponent(std::move(comp));
    return true;
}

bool DamageFeedbackHudSystem::triggerShieldHit(const std::string& entity_id,
    float intensity) {
    auto* fb = getComponentFor(entity_id);
    if (!fb) return false;
    fb->shield_feedback.intensity = (std::max)(fb->shield_feedback.intensity, (std::min)(1.0f, intensity));
    fb->shield_feedback.active = true;
    fb->shield_feedback.duration = 0.0f;
    fb->total_shield_hits++;
    return true;
}

bool DamageFeedbackHudSystem::triggerArmorHit(const std::string& entity_id,
    float intensity) {
    auto* fb = getComponentFor(entity_id);
    if (!fb) return false;
    fb->armor_feedback.intensity = (std::max)(fb->armor_feedback.intensity, (std::min)(1.0f, intensity));
    fb->armor_feedback.active = true;
    fb->armor_feedback.duration = 0.0f;
    fb->total_armor_hits++;
    return true;
}

bool DamageFeedbackHudSystem::triggerHullHit(const std::string& entity_id,
    float intensity) {
    auto* fb = getComponentFor(entity_id);
    if (!fb) return false;
    fb->hull_feedback.intensity = (std::max)(fb->hull_feedback.intensity, (std::min)(1.0f, intensity));
    fb->hull_feedback.active = true;
    fb->hull_feedback.duration = 0.0f;
    fb->screen_shake_intensity = (std::max)(fb->screen_shake_intensity, (std::min)(1.0f, intensity));
    fb->total_hull_hits++;
    return true;
}

float DamageFeedbackHudSystem::getShieldIntensity(const std::string& entity_id) const {
    auto* fb = getComponentFor(entity_id);
    return fb ? fb->shield_feedback.intensity : 0.0f;
}

float DamageFeedbackHudSystem::getArmorIntensity(const std::string& entity_id) const {
    auto* fb = getComponentFor(entity_id);
    return fb ? fb->armor_feedback.intensity : 0.0f;
}

float DamageFeedbackHudSystem::getHullIntensity(const std::string& entity_id) const {
    auto* fb = getComponentFor(entity_id);
    return fb ? fb->hull_feedback.intensity : 0.0f;
}

float DamageFeedbackHudSystem::getScreenShake(const std::string& entity_id) const {
    auto* fb = getComponentFor(entity_id);
    return fb ? fb->screen_shake_intensity : 0.0f;
}

int DamageFeedbackHudSystem::getTotalShieldHits(const std::string& entity_id) const {
    auto* fb = getComponentFor(entity_id);
    return fb ? fb->total_shield_hits : 0;
}

int DamageFeedbackHudSystem::getTotalArmorHits(const std::string& entity_id) const {
    auto* fb = getComponentFor(entity_id);
    return fb ? fb->total_armor_hits : 0;
}

int DamageFeedbackHudSystem::getTotalHullHits(const std::string& entity_id) const {
    auto* fb = getComponentFor(entity_id);
    return fb ? fb->total_hull_hits : 0;
}

bool DamageFeedbackHudSystem::setVisible(const std::string& entity_id, bool vis) {
    auto* fb = getComponentFor(entity_id);
    if (!fb) return false;
    fb->visible = vis;
    return true;
}

} // namespace systems
} // namespace atlas
