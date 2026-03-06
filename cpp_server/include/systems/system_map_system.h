#ifndef NOVAFORGE_SYSTEMS_SYSTEM_MAP_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SYSTEM_MAP_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief In-system map data for celestials, bookmarks, and signatures
 *
 * Manages the objects visible on the system map including celestial bodies,
 * player bookmarks, and scanned signatures with distance calculations.
 */
class SystemMapSystem : public ecs::SingleComponentSystem<components::SystemMap> {
public:
    explicit SystemMapSystem(ecs::World* world);
    ~SystemMapSystem() override = default;

    std::string getName() const override { return "SystemMapSystem"; }

    bool addCelestial(const std::string& entity_id, const std::string& celestial_id,
                      const std::string& name, const std::string& type,
                      float x, float y, float z, float radius);
    bool removeCelestial(const std::string& entity_id, const std::string& celestial_id);
    bool addBookmark(const std::string& entity_id, const std::string& bookmark_id,
                     const std::string& label, const std::string& folder,
                     float x, float y, float z);
    bool removeBookmark(const std::string& entity_id, const std::string& bookmark_id);
    bool addSignature(const std::string& entity_id, const std::string& sig_id,
                      const std::string& type, float scan_strength,
                      float x, float y, float z);
    bool resolveSignature(const std::string& entity_id, const std::string& sig_id);
    int getCelestialCount(const std::string& entity_id) const;
    int getBookmarkCount(const std::string& entity_id) const;
    int getSignatureCount(const std::string& entity_id) const;
    float getDistanceBetween(const std::string& entity_id,
                             const std::string& id_a, const std::string& id_b) const;
    int getTotalBookmarksCreated(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SystemMap& map, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SYSTEM_MAP_SYSTEM_H
