#include "characters/equipment_attachment_system.h"
#include <algorithm>

namespace atlas {

void EquipmentAttachmentSystem::attachModule(const EquipmentModule& module,
                                              AstronautCharacter& character) {
    modules.push_back(module);

    MeshPiece piece;
    piece.meshFile = module.meshFile;
    piece.material = "equipment_default";
    piece.scale = module.scale;
    piece.offset = module.offset;
    character.accessories.push_back(piece);
}

void EquipmentAttachmentSystem::removeModule(EquipmentType type,
                                              AstronautCharacter& character) {
    // Find and remove from internal list
    auto modIt = std::find_if(modules.begin(), modules.end(),
        [type](const EquipmentModule& m) { return m.type == type; });

    if (modIt == modules.end())
        return;

    std::string meshToRemove = modIt->meshFile;
    modules.erase(modIt);

    // Remove matching accessory from character
    auto accIt = std::find_if(character.accessories.begin(), character.accessories.end(),
        [&meshToRemove](const MeshPiece& p) { return p.meshFile == meshToRemove; });

    if (accIt != character.accessories.end()) {
        character.accessories.erase(accIt);
    }
}

void EquipmentAttachmentSystem::updateModulesForSliders(AstronautCharacter& character) {
    // Find relevant slider values
    float torsoWidth = 1.0f;
    float height = 1.0f;
    for (const auto& slider : character.sliders) {
        if (slider.name == "torsoWidth")
            torsoWidth = slider.currentValue;
        else if (slider.name == "height")
            height = slider.currentValue;
    }

    // Scale each equipment module and its matching accessory
    for (size_t i = 0; i < modules.size(); ++i) {
        auto& mod = modules[i];

        switch (mod.type) {
        case EquipmentType::Backpack:
        case EquipmentType::Suit:
            mod.scale = glm::vec3(torsoWidth, height, torsoWidth);
            break;
        case EquipmentType::Helmet:
            // Helmets only scale slightly with torso width
            mod.scale = glm::vec3(torsoWidth, torsoWidth, torsoWidth);
            break;
        default:
            break;
        }

        // Sync accessory
        auto accIt = std::find_if(character.accessories.begin(), character.accessories.end(),
            [&mod](const MeshPiece& p) { return p.meshFile == mod.meshFile; });
        if (accIt != character.accessories.end()) {
            accIt->scale = mod.scale;
        }
    }
}

int EquipmentAttachmentSystem::totalStorageSlots() const {
    int total = 0;
    for (const auto& mod : modules) {
        total += mod.storageSlots;
    }
    return total;
}

float EquipmentAttachmentSystem::totalProtection() const {
    float total = 0.0f;
    for (const auto& mod : modules) {
        total += mod.protection;
    }
    return total;
}

} // namespace atlas
