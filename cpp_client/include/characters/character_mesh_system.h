#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace atlas {

enum class BodyPart : uint32_t { Head, Torso, ArmLeft, ArmRight, LegLeft, LegRight };

struct BodySlider {
    std::string name;
    float minValue;
    float maxValue;
    float currentValue;
    BodyPart targetPart;
};

struct MeshPiece {
    std::string meshFile;
    std::string material;
    glm::vec3 scale;
    glm::vec3 offset;
};

struct AstronautCharacter {
    int characterId{0};
    bool isFemale{false};
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    std::vector<MeshPiece> bodyMeshes;
    std::vector<MeshPiece> accessories;
    std::vector<BodySlider> sliders;
};

class CharacterMeshSystem {
public:
    AstronautCharacter generateCharacter(int seed, bool isFemale);
    void applySlider(AstronautCharacter& character, const std::string& sliderName, float value);
    void assembleMeshes(AstronautCharacter& character);
};

} // namespace atlas
