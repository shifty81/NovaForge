#ifndef NOVAFORGE_SYSTEMS_MOD_DOC_GENERATOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MOD_DOC_GENERATOR_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Automated modding documentation generator
 *
 * Catalogs registered content types (ships, modules, missions, skills) with
 * their schemas, validation rules, and examples for mod authors.
 */
class ModDocGeneratorSystem : public ecs::System {
public:
    explicit ModDocGeneratorSystem(ecs::World* world);
    ~ModDocGeneratorSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ModDocGeneratorSystem"; }

    bool createGenerator(const std::string& entity_id);
    bool registerType(const std::string& entity_id, const std::string& type_name,
                      const std::string& category, const std::string& description,
                      int field_count);
    bool addExample(const std::string& entity_id, const std::string& type_name);
    bool validateEntry(const std::string& entity_id, const std::string& type_name);
    bool generate(const std::string& entity_id);
    int getEntryCount(const std::string& entity_id) const;
    int getEntriesByCategory(const std::string& entity_id, const std::string& category) const;
    bool isGenerated(const std::string& entity_id) const;
    int getValidatedCount(const std::string& entity_id) const;
    int getGenerationCount(const std::string& entity_id) const;
    bool setTitle(const std::string& entity_id, const std::string& title);
    bool setVersion(const std::string& entity_id, const std::string& version);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MOD_DOC_GENERATOR_SYSTEM_H
