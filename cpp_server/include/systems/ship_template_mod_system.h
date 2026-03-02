#ifndef NOVAFORGE_SYSTEMS_SHIP_TEMPLATE_MOD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_TEMPLATE_MOD_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship template mods allowing custom ships
 *
 * Manages moddable ship templates with inheritance from base templates,
 * priority-based overrides, and auto-validation.
 */
class ShipTemplateModSystem : public ecs::System {
public:
    explicit ShipTemplateModSystem(ecs::World* world);
    ~ShipTemplateModSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ShipTemplateModSystem"; }

    bool registerTemplate(const std::string& entity_id, const std::string& template_id,
                          const std::string& ship_name, const std::string& ship_class,
                          const std::string& faction);
    bool setBaseTemplate(const std::string& entity_id, const std::string& base_template_id);
    bool setModSource(const std::string& entity_id, const std::string& mod_source, int priority);
    bool validateTemplate(const std::string& entity_id);
    bool isValid(const std::string& entity_id) const;
    std::string getTemplateId(const std::string& entity_id) const;
    std::string getShipClass(const std::string& entity_id) const;
    int getHighestPriority() const;
    int getTemplateCount() const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_TEMPLATE_MOD_SYSTEM_H
