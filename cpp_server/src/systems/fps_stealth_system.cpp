#include "systems/fps_stealth_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FPSStealthSystem::FPSStealthSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FPSStealthSystem::updateComponent(ecs::Entity& /*entity*/,
    components::FPSStealth& stealth, float delta_time) {
    if (!stealth.active) return;

    // Calculate effective visibility based on stance, shadow, light
    float vis = stealth.light_level;
    if (stealth.is_crouching) vis *= stealth.crouch_visibility_mult;
    if (stealth.in_shadow) vis *= 0.3f;
    stealth.visibility = std::max(0.0f, std::min(1.0f, vis));

    // Calculate noise based on movement
    float noise = 0.1f;  // base noise
    if (stealth.is_sprinting) noise *= stealth.sprint_noise_mult;
    if (stealth.is_crouching) noise *= 0.3f;
    stealth.noise_level = std::max(0.0f, std::min(1.0f, noise));

    // Detection meter decay when not being detected
    int prev_state = static_cast<int>(stealth.state);
    stealth.detection_meter -= stealth.detection_decay_rate * delta_time;
    stealth.detection_meter = std::max(0.0f, stealth.detection_meter);

    // Update detection state based on meter thresholds
    if (stealth.detection_meter >= stealth.full_alert_threshold) {
        stealth.state = components::FPSStealth::FullAlert;
    } else if (stealth.detection_meter >= stealth.detected_threshold) {
        stealth.state = components::FPSStealth::Detected;
    } else if (stealth.detection_meter >= stealth.suspicious_threshold) {
        stealth.state = components::FPSStealth::Suspicious;
    } else {
        stealth.state = components::FPSStealth::Hidden;
    }

    // Track detection/escape transitions
    if (stealth.state >= components::FPSStealth::Detected &&
        prev_state < static_cast<int>(components::FPSStealth::Detected)) {
        stealth.times_detected++;
    }
    if (stealth.state == components::FPSStealth::Hidden &&
        prev_state >= static_cast<int>(components::FPSStealth::Detected)) {
        stealth.times_escaped++;
    }

    // Track hidden time
    if (stealth.state == components::FPSStealth::Hidden) {
        stealth.time_hidden += delta_time;
    }
}

int FPSStealthSystem::getDetectionState(const std::string& entity_id) const {
    auto* stealth = getComponentFor(entity_id);
    return stealth ? static_cast<int>(stealth->state) : 0;
}

float FPSStealthSystem::getDetectionMeter(const std::string& entity_id) const {
    auto* stealth = getComponentFor(entity_id);
    return stealth ? stealth->detection_meter : 0.0f;
}

bool FPSStealthSystem::addDetection(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* stealth = getComponentFor(entity_id);
    if (!stealth) return false;
    stealth->detection_meter = std::min(1.0f, stealth->detection_meter + amount);
    return true;
}

bool FPSStealthSystem::setCrouching(const std::string& entity_id, bool crouching) {
    auto* stealth = getComponentFor(entity_id);
    if (!stealth) return false;
    stealth->is_crouching = crouching;
    return true;
}

bool FPSStealthSystem::setSprinting(const std::string& entity_id, bool sprinting) {
    auto* stealth = getComponentFor(entity_id);
    if (!stealth) return false;
    stealth->is_sprinting = sprinting;
    return true;
}

bool FPSStealthSystem::setInShadow(const std::string& entity_id, bool in_shadow) {
    auto* stealth = getComponentFor(entity_id);
    if (!stealth) return false;
    stealth->in_shadow = in_shadow;
    return true;
}

bool FPSStealthSystem::setLightLevel(const std::string& entity_id, float level) {
    if (level < 0.0f || level > 1.0f) return false;
    auto* stealth = getComponentFor(entity_id);
    if (!stealth) return false;
    stealth->light_level = level;
    return true;
}

float FPSStealthSystem::getVisibility(const std::string& entity_id) const {
    auto* stealth = getComponentFor(entity_id);
    return stealth ? stealth->visibility : 0.0f;
}

float FPSStealthSystem::getNoiseLevel(const std::string& entity_id) const {
    auto* stealth = getComponentFor(entity_id);
    return stealth ? stealth->noise_level : 0.0f;
}

int FPSStealthSystem::getTimesDetected(const std::string& entity_id) const {
    auto* stealth = getComponentFor(entity_id);
    return stealth ? stealth->times_detected : 0;
}

int FPSStealthSystem::getTimesEscaped(const std::string& entity_id) const {
    auto* stealth = getComponentFor(entity_id);
    return stealth ? stealth->times_escaped : 0;
}

float FPSStealthSystem::getTimeHidden(const std::string& entity_id) const {
    auto* stealth = getComponentFor(entity_id);
    return stealth ? stealth->time_hidden : 0.0f;
}

bool FPSStealthSystem::resetDetection(const std::string& entity_id) {
    auto* stealth = getComponentFor(entity_id);
    if (!stealth) return false;
    stealth->detection_meter = 0.0f;
    // Don't set state directly — let updateComponent handle the transition
    // so that escape tracking works correctly
    return true;
}

} // namespace systems
} // namespace atlas
