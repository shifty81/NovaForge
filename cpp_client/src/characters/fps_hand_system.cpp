#include "characters/fps_hand_system.h"

namespace atlas {

void FPSHandSystem::initialize(const AstronautCharacter& /*character*/) {
    // Default left hand
    leftHand.mesh = {"Hand_Left_FPS.obj", "glove_base", glm::vec3(1.0f), glm::vec3(-0.25f, -0.15f, 0.4f)};
    leftHand.grabSocketLocal = glm::vec3(-0.03f, 0.0f, 0.06f);
    leftHand.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // Default right hand
    rightHand.mesh = {"Hand_Right_FPS.obj", "glove_base", glm::vec3(1.0f), glm::vec3(0.25f, -0.15f, 0.4f)};
    rightHand.grabSocketLocal = glm::vec3(0.03f, 0.0f, 0.06f);
    rightHand.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

void FPSHandSystem::updateGrabSockets(const AstronautCharacter& character) {
    // Scale socket position based on the character's arm length slider
    float armScale = 1.0f;
    for (const auto& slider : character.sliders) {
        if (slider.name == "armLength") {
            armScale = slider.currentValue;
            break;
        }
    }

    leftHand.grabSocketLocal = glm::vec3(-0.03f, 0.0f, 0.06f) * armScale;
    rightHand.grabSocketLocal = glm::vec3(0.03f, 0.0f, 0.06f) * armScale;
}

void FPSHandSystem::attachTool(const std::string& toolMesh, bool isLeftHand) {
    FPSHand& hand = isLeftHand ? leftHand : rightHand;
    hand.mesh.meshFile = toolMesh;
}

} // namespace atlas
