#include "systems/sector_map_discovery_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/exploration_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using SMD = components::SectorMapDiscovery;
using Sector = components::SectorMapDiscovery::DiscoveredSector;

Sector* findSector(SMD* smd, const std::string& sector_id) {
    for (auto& s : smd->sectors) {
        if (s.sector_id == sector_id) return &s;
    }
    return nullptr;
}

const Sector* findSectorConst(const SMD* smd, const std::string& sector_id) {
    for (const auto& s : smd->sectors) {
        if (s.sector_id == sector_id) return &s;
    }
    return nullptr;
}

} // anonymous namespace

SectorMapDiscoverySystem::SectorMapDiscoverySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SectorMapDiscoverySystem::updateComponent(ecs::Entity& entity,
    components::SectorMapDiscovery& smd, float delta_time) {
    if (!smd.active) return;
    smd.elapsed += delta_time;
}

bool SectorMapDiscoverySystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SectorMapDiscovery>();
    entity->addComponent(std::move(comp));
    return true;
}

bool SectorMapDiscoverySystem::discoverSector(const std::string& entity_id,
    const std::string& sector_id, const std::string& sector_name) {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return false;
    if (static_cast<int>(smd->sectors.size()) >= smd->max_sectors) return false;
    if (findSector(smd, sector_id)) return false;

    Sector s;
    s.sector_id = sector_id;
    s.sector_name = sector_name;
    s.visibility = 1;  // partial visibility on first discovery
    s.visit_count = 0;
    s.discovery_time = smd->elapsed;
    smd->sectors.push_back(s);
    smd->total_discoveries++;
    return true;
}

bool SectorMapDiscoverySystem::visitSector(const std::string& entity_id,
    const std::string& sector_id) {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return false;

    auto* sector = findSector(smd, sector_id);
    if (!sector) return false;

    sector->visit_count++;
    smd->total_visits++;
    if (sector->visibility < 2) {
        sector->visibility = 2;  // full visibility after first visit
    }
    return true;
}

bool SectorMapDiscoverySystem::setVisibility(const std::string& entity_id,
    const std::string& sector_id, int level) {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return false;

    auto* sector = findSector(smd, sector_id);
    if (!sector) return false;

    sector->visibility = std::max(0, std::min(2, level));
    return true;
}

bool SectorMapDiscoverySystem::removeSector(const std::string& entity_id,
    const std::string& sector_id) {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return false;

    auto it = std::find_if(smd->sectors.begin(), smd->sectors.end(),
        [&](const Sector& s) { return s.sector_id == sector_id; });
    if (it == smd->sectors.end()) return false;
    smd->sectors.erase(it);
    return true;
}

int SectorMapDiscoverySystem::getSectorCount(const std::string& entity_id) const {
    auto* smd = getComponentFor(entity_id);
    return smd ? static_cast<int>(smd->sectors.size()) : 0;
}

int SectorMapDiscoverySystem::getVisibility(const std::string& entity_id,
    const std::string& sector_id) const {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return 0;
    const auto* sector = findSectorConst(smd, sector_id);
    return sector ? sector->visibility : 0;
}

int SectorMapDiscoverySystem::getVisitCount(const std::string& entity_id,
    const std::string& sector_id) const {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return 0;
    const auto* sector = findSectorConst(smd, sector_id);
    return sector ? sector->visit_count : 0;
}

bool SectorMapDiscoverySystem::isSectorDiscovered(const std::string& entity_id,
    const std::string& sector_id) const {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return false;
    return findSectorConst(smd, sector_id) != nullptr;
}

int SectorMapDiscoverySystem::getFullyExploredCount(const std::string& entity_id) const {
    auto* smd = getComponentFor(entity_id);
    if (!smd) return 0;
    int count = 0;
    for (const auto& s : smd->sectors) {
        if (s.visibility >= 2) count++;
    }
    return count;
}

float SectorMapDiscoverySystem::getExplorationPercent(const std::string& entity_id) const {
    auto* smd = getComponentFor(entity_id);
    if (!smd || smd->max_sectors <= 0) return 0.0f;
    return static_cast<float>(smd->sectors.size()) / static_cast<float>(smd->max_sectors);
}

int SectorMapDiscoverySystem::getTotalVisits(const std::string& entity_id) const {
    auto* smd = getComponentFor(entity_id);
    return smd ? smd->total_visits : 0;
}

} // namespace systems
} // namespace atlas
