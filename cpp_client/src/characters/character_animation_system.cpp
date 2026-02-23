#include "characters/character_animation_system.h"

namespace atlas {

void CharacterAnimationSystem::playAnimation(AstronautCharacter& /*character*/,
                                              AnimationType type) {
    m_currentAnimation = type;
    m_blendFactor = 1.0f;
}

void CharacterAnimationSystem::blendAnimation(AstronautCharacter& /*character*/,
                                               AnimationType type, float blendFactor) {
    m_currentAnimation = type;
    m_blendFactor = blendFactor;
}

void CharacterAnimationSystem::updateFPSInteractionIK(const Skeleton& skeleton,
                                                       const IKTarget& handTarget,
                                                       bool isLeftHand) {
    // Basic position offset computation for FPS hand IK
    if (skeleton.bones.empty())
        return;

    // Determine hand bone index (Hand_L = 9, Hand_R = 13 in the base skeleton)
    size_t handBoneIndex = isLeftHand ? 9 : 13;
    if (handBoneIndex >= skeleton.bones.size())
        return;

    // Compute offset from current hand bone position to the IK target
    const auto& handBone = skeleton.bones[handBoneIndex];
    glm::vec3 offset = handTarget.position - handBone.localPosition;
    (void)offset; // Stub: offset would be applied by the animation blending pipeline
}

} // namespace atlas
