#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "characters/character_mesh_system.h"

namespace atlas {

enum class EquipmentType : uint32_t { Backpack, Suit, Helmet, Tool, Weapon };

struct EquipmentModule {
    EquipmentType type{EquipmentType::Tool};
    std::string meshFile;
    glm::vec3 offset{0.0f};
    glm::vec3 scale{1.0f};
    int storageSlots{0};
    float protection{0.0f};
    float powerConsumption{0.0f};
};

class EquipmentAttachmentSystem {
public:
    std::vector<EquipmentModule> modules;

    void attachModule(const EquipmentModule& module, AstronautCharacter& character);
    void removeModule(EquipmentType type, AstronautCharacter& character);
    void updateModulesForSliders(AstronautCharacter& character);
    int totalStorageSlots() const;
    float totalProtection() const;
};

} // namespace atlas
