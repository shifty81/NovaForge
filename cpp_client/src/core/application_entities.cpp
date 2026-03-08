#include "core/application.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/solar_system_scene.h"
#include "ui/entity_picker.h"
#include "ui/atlas/atlas_hud.h"
#include "rendering/window.h"
#include "rendering/camera.h"
#include <iostream>
#include <algorithm>

namespace atlas {

void Application::targetEntity(const std::string& entityId, bool addToTargets) {
    if (entityId.empty()) {
        return;
    }
    
    std::cout << "[Targeting] Target entity: " << entityId;
    if (addToTargets) {
        std::cout << " (add to targets)";
    }
    std::cout << std::endl;
    
    if (addToTargets) {
        // Add to target list if not already present
        auto it = std::find(m_targetList.begin(), m_targetList.end(), entityId);
        if (it == m_targetList.end()) {
            m_targetList.push_back(entityId);
            // Send target lock request to server
            auto* networkMgr = m_gameClient->getNetworkManager();
            if (networkMgr && networkMgr->isConnected()) {
                networkMgr->sendTargetLock(entityId);
            }
        }
    } else {
        // Replace current target
        m_currentTargetId = entityId;
        m_targetList.clear();
        m_targetList.push_back(entityId);
        m_currentTargetIndex = 0;
        // Send target lock request to server
        auto* networkMgr = m_gameClient->getNetworkManager();
        if (networkMgr && networkMgr->isConnected()) {
            networkMgr->sendTargetLock(entityId);
        }
    }
}

void Application::clearTarget() {
    std::cout << "[Targeting] Clear target" << std::endl;
    
    // Send target unlock requests to server
    auto* networkMgr = m_gameClient->getNetworkManager();
    if (networkMgr && networkMgr->isConnected()) {
        for (const auto& targetId : m_targetList) {
            networkMgr->sendTargetUnlock(targetId);
        }
    }

    m_currentTargetId.clear();
    m_targetList.clear();
    m_currentTargetIndex = -1;

}

void Application::cycleTarget() {
    if (m_targetList.empty()) {
        std::cout << "[Targeting] No targets to cycle" << std::endl;
        return;
    }
    
    // Move to next target
    m_currentTargetIndex = (m_currentTargetIndex + 1) % m_targetList.size();
    m_currentTargetId = m_targetList[m_currentTargetIndex];
    
    std::cout << "[Targeting] Cycle to target: " << m_currentTargetId 
              << " (" << (m_currentTargetIndex + 1) << "/" << m_targetList.size() << ")" << std::endl;
}

void Application::activateModule(int slotNumber) {
    if (slotNumber < 1 || slotNumber > 8) {
        return;
    }
    
    std::cout << "[Modules] Activate module in slot " << slotNumber;
    if (!m_currentTargetId.empty()) {
        std::cout << " on target: " << m_currentTargetId;
    }
    std::cout << std::endl;
    
    // Send module activation command to server with current target
    auto* networkMgr = m_gameClient->getNetworkManager();
    if (networkMgr && networkMgr->isConnected()) {
        networkMgr->sendModuleActivate(slotNumber - 1, m_currentTargetId);  // Convert to 0-based index
    } else {
        std::cout << "[Modules] Not connected to server, activation not sent" << std::endl;
    }
}

void Application::showSpaceContextMenu(double x, double y) {
    // Check if clicking on an entity
    auto entities = m_gameClient->getEntityManager().getAllEntities();
    std::vector<std::shared_ptr<Entity>> entityList;
    for (const auto& pair : entities) {
        if (pair.first != m_localPlayerId) {
            entityList.push_back(pair.second);
        }
    }
    
    std::string pickedId = m_entityPicker->pickEntity(
        x, y, m_window->getWidth(), m_window->getHeight(),
        *m_camera, entityList);
    
    m_contextMenuEntityId = pickedId;
    m_contextMenuX = x;
    m_contextMenuY = y;
    m_showContextMenu = true;
}

void Application::showEntityContextMenu(const std::string& entityId, double x, double y) {
    m_contextMenuEntityId = entityId;
    m_contextMenuX = x;
    m_contextMenuY = y;
    m_showContextMenu = true;
}

void Application::openInfoPanelForEntity(const std::string& entityId) {
    auto entity = m_gameClient->getEntityManager().getEntity(entityId);
    if (entity && m_atlasHUD) {
        auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
        atlas::InfoPanelData info;
        info.name = entity->getShipName().empty() ? entityId : entity->getShipName();
        info.type = entity->getShipType();
        info.faction = entity->getFaction();
        const auto& health = entity->getHealth();
        info.shieldPct = health.maxShield > 0 ? health.currentShield / static_cast<float>(health.maxShield) : 0.0f;
        info.armorPct = health.maxArmor > 0 ? health.currentArmor / static_cast<float>(health.maxArmor) : 0.0f;
        info.hullPct = health.maxHull > 0 ? health.currentHull / static_cast<float>(health.maxHull) : 0.0f;
        info.hasHealth = true;
        if (playerEntity) {
            info.distance = glm::distance(playerEntity->getPosition(), entity->getPosition());
        }
        m_atlasHUD->showInfoPanel(info);
        return;
    }

    // Check if it's a celestial object instead
    if (m_solarSystem && m_atlasHUD) {
        const auto* cel = m_solarSystem->findCelestial(entityId);
        if (cel) {
            atlas::InfoPanelData info;
            info.name = cel->name;
            switch (cel->type) {
                case atlas::Celestial::Type::PLANET:        info.type = "Planet";        break;
                case atlas::Celestial::Type::MOON:          info.type = "Moon";          break;
                case atlas::Celestial::Type::STATION:       info.type = "Station";       break;
                case atlas::Celestial::Type::STARGATE:      info.type = "Stargate";      break;
                case atlas::Celestial::Type::ASTEROID_BELT: info.type = "Asteroid Belt"; break;
                case atlas::Celestial::Type::WORMHOLE:      info.type = "Wormhole";      break;
                case atlas::Celestial::Type::ANOMALY:       info.type = cel->anomalyType.empty() ? "Anomaly" : cel->anomalyType; break;
                default:                                  info.type = "Celestial";     break;
            }
            info.hasHealth = false;
            auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
            if (playerEntity) {
                info.distance = glm::distance(playerEntity->getPosition(), cel->position);
            }
            m_atlasHUD->showInfoPanel(info);
        }
    }
}

void Application::spawnLocalPlayerEntity() {
    if (m_localPlayerSpawned) return;
    
    std::cout << "[PVE] Spawning local player ship..." << std::endl;
    
    // Create player entity at origin with a Fang (Keldari frigate)
    Health playerHealth(1500, 800, 500);  // Shield, Armor, Hull
    Capacitor playerCapacitor(250.0f, 250.0f);  // Fang capacitor: 250 GJ
    
    m_gameClient->getEntityManager().spawnEntity(
        m_localPlayerId,
        glm::vec3(0.0f, 0.0f, 0.0f),
        playerHealth,
        playerCapacitor,
        "Fang",
        "Your Ship",
        "Keldari"
    );
    
    m_localPlayerSpawned = true;
    std::cout << "[PVE] Local player ship spawned as Fang" << std::endl;
}

void Application::spawnDemoNPCEntities() {
    std::cout << "[PVE] Spawning demo NPC entities..." << std::endl;
    
    // Spawn some NPC enemies for the PVE demo
    // These would normally come from the server in missions/anomalies
    
    // Crimson Order pirate (hostile NPC)
    Health npc1Health(800, 600, 400);
    Capacitor npc1Cap(500.0f, 500.0f);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_raider_1",
        glm::vec3(300.0f, 10.0f, 200.0f),
        npc1Health,
        npc1Cap,
        "Cruiser",
        "Crimson Order",
        "Crimson Order"
    );
    
    // Venom Syndicate frigate
    Health npc2Health(400, 300, 200);
    Capacitor npc2Cap(300.0f, 300.0f);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_serp_1",
        glm::vec3(-250.0f, -5.0f, 350.0f),
        npc2Health,
        npc2Cap,
        "Frigate",
        "Venom Syndicate Scout",
        "Venom Syndicate"
    );
    
    // Iron Corsairs destroyer
    Health npc3Health(600, 500, 350);
    Capacitor npc3Cap(400.0f, 400.0f);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_gur_1",
        glm::vec3(150.0f, 20.0f, -300.0f),
        npc3Health,
        npc3Cap,
        "Destroyer",
        "Iron Corsairs Watchman",
        "Iron Corsairs"
    );
    
    std::cout << "[PVE] 3 NPC entities spawned" << std::endl;
}

} // namespace atlas
