#pragma once

#include <string>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "characters/character_mesh_system.h"
#include "characters/character_skeleton.h"

namespace atlas {

enum class AnimationType : uint32_t {
    Idle, Walk, Run, Crouch, Float, PushButton, PullLever, GrabItem
};

struct AnimationClip {
    std::string name;
    float duration{0.0f};
    AnimationType type{AnimationType::Idle};
};

struct IKTarget {
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
};

class CharacterAnimationSystem {
public:
    void playAnimation(AstronautCharacter& character, AnimationType type);
    void blendAnimation(AstronautCharacter& character, AnimationType type, float blendFactor);
    void updateFPSInteractionIK(const Skeleton& skeleton, const IKTarget& handTarget, bool isLeftHand);

private:
    AnimationType m_currentAnimation{AnimationType::Idle};
    float m_blendFactor{0.0f};
};

} // namespace atlas
