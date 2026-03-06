#include "systems/overview_filter_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

OverviewFilterSystem::OverviewFilterSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OverviewFilterSystem::updateComponent(ecs::Entity& /*entity*/,
    components::OverviewFilter& overview, float delta_time) {
    if (!overview.active) return;

    overview.update_timer += delta_time;
    if (overview.update_timer >= overview.update_interval) {
        overview.update_timer = 0.0f;

        // Sort entries based on current sort column
        if (overview.sort_column == "distance") {
            std::sort(overview.entries.begin(), overview.entries.end(),
                [&overview](const components::OverviewFilter::OverviewEntry& a,
                            const components::OverviewFilter::OverviewEntry& b) {
                    return overview.sort_ascending ? a.distance < b.distance : a.distance > b.distance;
                });
        } else if (overview.sort_column == "name") {
            std::sort(overview.entries.begin(), overview.entries.end(),
                [&overview](const components::OverviewFilter::OverviewEntry& a,
                            const components::OverviewFilter::OverviewEntry& b) {
                    return overview.sort_ascending ? a.name < b.name : a.name > b.name;
                });
        } else if (overview.sort_column == "type") {
            std::sort(overview.entries.begin(), overview.entries.end(),
                [&overview](const components::OverviewFilter::OverviewEntry& a,
                            const components::OverviewFilter::OverviewEntry& b) {
                    return overview.sort_ascending ? a.type < b.type : a.type > b.type;
                });
        }

        overview.total_entries_filtered++;
    }
}

bool OverviewFilterSystem::addPreset(const std::string& entity_id,
    const std::string& preset_id, const std::string& name) {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;

    for (const auto& p : overview->presets) {
        if (p.preset_id == preset_id) return false;
    }
    if (static_cast<int>(overview->presets.size()) >= overview->max_presets) return false;

    components::OverviewFilter::FilterPreset preset;
    preset.preset_id = preset_id;
    preset.name = name;
    overview->presets.push_back(preset);
    return true;
}

bool OverviewFilterSystem::removePreset(const std::string& entity_id,
    const std::string& preset_id) {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;

    auto it = std::remove_if(overview->presets.begin(), overview->presets.end(),
        [&preset_id](const components::OverviewFilter::FilterPreset& p) {
            return p.preset_id == preset_id;
        });
    if (it == overview->presets.end()) return false;
    overview->presets.erase(it, overview->presets.end());
    if (overview->active_preset_id == preset_id) {
        overview->active_preset_id.clear();
    }
    return true;
}

bool OverviewFilterSystem::setActivePreset(const std::string& entity_id,
    const std::string& preset_id) {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;

    for (const auto& p : overview->presets) {
        if (p.preset_id == preset_id) {
            overview->active_preset_id = preset_id;
            return true;
        }
    }
    return false;
}

bool OverviewFilterSystem::addTypeToPreset(const std::string& entity_id,
    const std::string& preset_id, const std::string& type) {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;

    for (auto& p : overview->presets) {
        if (p.preset_id == preset_id) {
            for (const auto& t : p.shown_types) {
                if (t == type) return false;  // already exists
            }
            p.shown_types.push_back(type);
            return true;
        }
    }
    return false;
}

bool OverviewFilterSystem::setPresetMaxDistance(const std::string& entity_id,
    const std::string& preset_id, float max_distance) {
    if (max_distance < 0.0f) return false;
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;

    for (auto& p : overview->presets) {
        if (p.preset_id == preset_id) {
            p.max_distance = max_distance;
            return true;
        }
    }
    return false;
}

bool OverviewFilterSystem::addEntry(const std::string& entity_id,
    const std::string& entry_entity_id, const std::string& name,
    const std::string& type, float distance, const std::string& standing) {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;

    for (const auto& e : overview->entries) {
        if (e.entity_id == entry_entity_id) return false;
    }
    if (static_cast<int>(overview->entries.size()) >= overview->max_entries) return false;

    components::OverviewFilter::OverviewEntry entry;
    entry.entity_id = entry_entity_id;
    entry.name = name;
    entry.type = type;
    entry.distance = distance;
    entry.standing = standing;
    overview->entries.push_back(entry);
    return true;
}

bool OverviewFilterSystem::removeEntry(const std::string& entity_id,
    const std::string& entry_entity_id) {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;

    auto it = std::remove_if(overview->entries.begin(), overview->entries.end(),
        [&entry_entity_id](const components::OverviewFilter::OverviewEntry& e) {
            return e.entity_id == entry_entity_id;
        });
    if (it == overview->entries.end()) return false;
    overview->entries.erase(it, overview->entries.end());
    return true;
}

int OverviewFilterSystem::getEntryCount(const std::string& entity_id) const {
    auto* overview = getComponentFor(entity_id);
    return overview ? static_cast<int>(overview->entries.size()) : 0;
}

int OverviewFilterSystem::getPresetCount(const std::string& entity_id) const {
    auto* overview = getComponentFor(entity_id);
    return overview ? static_cast<int>(overview->presets.size()) : 0;
}

bool OverviewFilterSystem::setSortColumn(const std::string& entity_id,
    const std::string& column, bool ascending) {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return false;
    if (column != "name" && column != "type" && column != "distance" && column != "angular_velocity") return false;
    overview->sort_column = column;
    overview->sort_ascending = ascending;
    return true;
}

int OverviewFilterSystem::getFilteredEntryCount(const std::string& entity_id) const {
    auto* overview = getComponentFor(entity_id);
    if (!overview) return 0;

    if (overview->active_preset_id.empty()) {
        return static_cast<int>(overview->entries.size());
    }

    // Find active preset
    const components::OverviewFilter::FilterPreset* active = nullptr;
    for (const auto& p : overview->presets) {
        if (p.preset_id == overview->active_preset_id) {
            active = &p;
            break;
        }
    }
    if (!active) return static_cast<int>(overview->entries.size());

    int count = 0;
    for (const auto& e : overview->entries) {
        // Type filter
        if (!active->shown_types.empty()) {
            bool found = false;
            for (const auto& t : active->shown_types) {
                if (t == e.type) { found = true; break; }
            }
            if (!found) continue;
        }
        // Distance filter
        if (active->max_distance > 0.0f && e.distance > active->max_distance) continue;
        // Standing filter
        if (!active->show_friendly && e.standing == "Friendly") continue;
        if (!active->show_neutral && e.standing == "Neutral") continue;
        if (!active->show_hostile && e.standing == "Hostile") continue;
        count++;
    }
    return count;
}

int OverviewFilterSystem::getTotalEntriesFiltered(const std::string& entity_id) const {
    auto* overview = getComponentFor(entity_id);
    return overview ? overview->total_entries_filtered : 0;
}

} // namespace systems
} // namespace atlas
