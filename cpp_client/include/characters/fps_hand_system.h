#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "characters/character_mesh_system.h"

namespace atlas {

struct FPSHand {
    MeshPiece mesh;
    glm::vec3 grabSocketLocal{0.0f};
    glm::quat orientation{1.0f, 0.0f, 0.0f, 0.0f};
};

class FPSHandSystem {
public:
    FPSHand leftHand;
    FPSHand rightHand;

    void initialize(const AstronautCharacter& character);
    void updateGrabSockets(const AstronautCharacter& character);
    void attachTool(const std::string& toolMesh, bool isLeftHand);
};

} // namespace atlas
