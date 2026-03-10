#ifndef NOVAFORGE_SYSTEMS_CORPORATION_LOGO_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CORPORATION_LOGO_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Corporation logo (emblem) layer-stack management system.
 *
 * A corporation logo is composed of up to max_layers stacked LogoLayers
 * (Background, Foreground, Overlay).  Layers can be added, removed, and
 * edited.  One layer is optionally designated the "active" layer for
 * quick in-place edits (color, opacity, scale, offsets).  The logo can
 * be published (locked for display); publishing prevents further edits
 * until the logo is reset.
 */
class CorporationLogoSystem
    : public ecs::SingleComponentSystem<components::CorporationLogo> {
public:
    explicit CorporationLogoSystem(ecs::World* world);
    ~CorporationLogoSystem() override = default;

    std::string getName() const override { return "CorporationLogoSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& corp_id,
                    const std::string& corp_name);

    // --- Layer management ---
    bool addLayer(const std::string& entity_id,
                  const std::string& layer_id,
                  const std::string& name,
                  components::CorporationLogo::LayerType type,
                  const std::string& color,
                  float opacity,
                  float scale);
    bool removeLayer(const std::string& entity_id,
                     const std::string& layer_id);
    bool setActiveLayer(const std::string& entity_id,
                        const std::string& layer_id);

    // --- Active layer editing ---
    bool setLayerColor(const std::string& entity_id,
                       const std::string& color);
    bool setLayerOpacity(const std::string& entity_id, float opacity);
    bool setLayerScale(const std::string& entity_id, float scale);
    bool setLayerOffset(const std::string& entity_id,
                        float offset_x, float offset_y);

    // --- Publication ---
    bool publishLogo(const std::string& entity_id);
    bool resetLogo(const std::string& entity_id);

    // --- Queries ---
    int         getLayerCount(const std::string& entity_id) const;
    std::string getActiveLayerId(const std::string& entity_id) const;
    bool        isPublished(const std::string& entity_id) const;
    int         getTotalEdits(const std::string& entity_id) const;
    bool        hasLayer(const std::string& entity_id,
                         const std::string& layer_id) const;
    std::string getCorpName(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CorporationLogo& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CORPORATION_LOGO_SYSTEM_H
