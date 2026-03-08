#include "core/application.h"
#include "rendering/camera.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/embedded_server.h"
#include "core/session_manager.h"
#include "core/solar_system_scene.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace atlas {

bool Application::hostMultiplayerGame(const std::string& sessionName, int maxPlayers) {
    std::cout << "Hosting multiplayer game: " << sessionName << std::endl;
    
    // Configure embedded server
    EmbeddedServer::Config serverConfig;
    serverConfig.server_name = sessionName;
    serverConfig.description = "Nova Forge Hosted Game";
    serverConfig.port = 8765;
    serverConfig.max_players = maxPlayers;
    serverConfig.lan_only = true;
    serverConfig.persistent_world = false;
    
    // Start embedded server
    if (!m_embeddedServer->start(serverConfig)) {
        std::cerr << "Failed to start embedded server!" << std::endl;
        return false;
    }
    
    // Configure session
    SessionManager::SessionConfig sessionConfig;
    sessionConfig.session_name = sessionName;
    sessionConfig.max_players = maxPlayers;
    sessionConfig.lan_only = true;
    
    // Host session
    if (!m_sessionManager->hostSession(sessionConfig, m_embeddedServer.get())) {
        std::cerr << "Failed to host session!" << std::endl;
        m_embeddedServer->stop();
        return false;
    }
    
    // Auto-connect to own server
    std::string localAddress = m_embeddedServer->getLocalAddress();
    int port = m_embeddedServer->getPort();
    
    // Give server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    if (!m_gameClient->connect(localAddress, port)) {
        std::cerr << "Failed to connect to own server!" << std::endl;
        m_sessionManager->leaveSession();
        m_embeddedServer->stop();
        return false;
    }
    
    std::cout << "Successfully hosting multiplayer game!" << std::endl;
    std::cout << "Other players can connect to: " << localAddress << ":" << port << std::endl;
    
    return true;
}

bool Application::joinMultiplayerGame(const std::string& host, int port) {
    std::cout << "Joining multiplayer game at " << host << ":" << port << std::endl;
    
    // Connect to remote server
    if (!m_gameClient->connect(host, port)) {
        std::cerr << "Failed to connect to server!" << std::endl;
        return false;
    }
    
    // Join session
    if (!m_sessionManager->joinSession(host, port)) {
        std::cerr << "Failed to join session!" << std::endl;
        m_gameClient->disconnect();
        return false;
    }
    
    std::cout << "Successfully joined multiplayer game!" << std::endl;
    return true;
}

bool Application::isHosting() const {
    return m_embeddedServer && m_embeddedServer->isRunning();
}

const char* Application::gameStateName(GameState state) {
    switch (state) {
        case GameState::InSpace:          return "InSpace";
        case GameState::Docking:          return "Docking";
        case GameState::Docked:           return "Docked";
        case GameState::StationInterior:  return "StationInterior";
        case GameState::ShipInterior:     return "ShipInterior";
        case GameState::Cockpit:          return "Cockpit";
    }
    return "Unknown";
}

void Application::requestStateTransition(GameState newState) {
    if (newState == m_gameState) return;

    std::cout << "[GameState] " << gameStateName(m_gameState)
              << " -> " << gameStateName(newState) << std::endl;

    // Validate allowed transitions
    bool valid = false;
    switch (m_gameState) {
        case GameState::InSpace:
            valid = (newState == GameState::Docking || newState == GameState::Cockpit);
            break;
        case GameState::Docking:
            valid = (newState == GameState::Docked || newState == GameState::InSpace);
            break;
        case GameState::Docked:
            valid = (newState == GameState::InSpace ||
                     newState == GameState::StationInterior ||
                     newState == GameState::ShipInterior ||
                     newState == GameState::Cockpit);
            break;
        case GameState::StationInterior:
            valid = (newState == GameState::Docked || newState == GameState::ShipInterior);
            break;
        case GameState::ShipInterior:
            valid = (newState == GameState::Docked ||
                     newState == GameState::StationInterior ||
                     newState == GameState::Cockpit);
            break;
        case GameState::Cockpit:
            valid = (newState == GameState::InSpace ||
                     newState == GameState::ShipInterior ||
                     newState == GameState::Docked);
            break;
    }

    if (!valid) {
        std::cout << "[GameState] Invalid transition — ignored" << std::endl;
        return;
    }

    m_gameState = newState;

    // ── Cursor capture: lock cursor in FPS modes, release in others ──
    if (isInFPSMode()) {
        captureFPSCursor();
    } else {
        releaseFPSCursor();
    }

    // Apply camera mode appropriate for the new state
    switch (m_gameState) {
        case GameState::InSpace:
            m_camera->setViewMode(atlas::ViewMode::ORBIT);
            m_activeModeText.clear();
            break;
        case GameState::Docking:
            m_camera->setViewMode(atlas::ViewMode::ORBIT);
            m_dockingTimer = DOCKING_ANIM_DURATION;
            m_activeModeText = "DOCKING";
            break;
        case GameState::Docked:
            m_camera->setViewMode(atlas::ViewMode::FPS);
            // Place the player at eye level on the hangar catwalk,
            // looking toward the landing pad and ship.
            m_camera->setFPSPosition(glm::vec3(-30.0f, 1.8f, 0.0f),
                                      glm::vec3(1.0f, 0.0f, 0.0f));
            m_activeModeText = "DOCKED";
            break;
        case GameState::StationInterior:
            m_camera->setViewMode(atlas::ViewMode::FPS);
            m_activeModeText = "STATION INTERIOR";
            break;
        case GameState::ShipInterior:
            m_camera->setViewMode(atlas::ViewMode::FPS);
            m_activeModeText = "SHIP INTERIOR";
            break;
        case GameState::Cockpit:
            m_camera->setViewMode(atlas::ViewMode::COCKPIT);
            m_activeModeText = "COCKPIT";
            break;
    }
}

void Application::toggleViewMode() {
    switch (m_gameState) {
        case GameState::InSpace:
            // ORBIT ↔ COCKPIT
            if (m_camera->getViewMode() == atlas::ViewMode::ORBIT) {
                requestStateTransition(GameState::Cockpit);
            } else {
                requestStateTransition(GameState::InSpace);
            }
            break;
        case GameState::Docked:
        case GameState::ShipInterior:
            // FPS ↔ COCKPIT
            if (m_camera->getViewMode() == atlas::ViewMode::COCKPIT) {
                requestStateTransition(GameState::ShipInterior);
            } else {
                requestStateTransition(GameState::Cockpit);
            }
            break;
        case GameState::StationInterior:
            // Must board ship first — enter ship interior, then cockpit
            std::cout << "[ViewMode] Board your ship first to enter the cockpit" << std::endl;
            break;
        case GameState::Cockpit:
            // Return to the previous logical state
            if (m_dockedStationId.empty()) {
                requestStateTransition(GameState::InSpace);
            } else {
                requestStateTransition(GameState::ShipInterior);
            }
            break;
        default:
            break;
    }
}

void Application::requestDock() {
    if (m_gameState != GameState::InSpace) {
        std::cout << "[Docking] Can only dock when in space" << std::endl;
        return;
    }

    if (!m_solarSystem) return;

    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (!playerEntity) return;

    glm::vec3 playerPos = playerEntity->getPosition();

    // Find the nearest station within docking range
    for (const auto& c : m_solarSystem->getCelestials()) {
        if (c.type != atlas::Celestial::Type::STATION) continue;
        if (m_solarSystem->isInDockingRange(playerPos, c.id)) {
            m_dockedStationId = c.id;
            commandStopShip();
            requestStateTransition(GameState::Docking);
            std::cout << "[Docking] Docking at " << c.name << std::endl;
            return;
        }
    }

    std::cout << "[Docking] No station within docking range" << std::endl;
}

void Application::requestUndock() {
    if (m_gameState != GameState::Docked &&
        m_gameState != GameState::StationInterior &&
        m_gameState != GameState::ShipInterior &&
        m_gameState != GameState::Cockpit) {
        std::cout << "[Undock] Not docked" << std::endl;
        return;
    }

    std::cout << "[Undock] Undocking from " << m_dockedStationId << std::endl;

    // Place ship outside station
    if (m_solarSystem) {
        const auto* station = m_solarSystem->findCelestial(m_dockedStationId);
        if (station) {
            glm::vec3 undockPos = station->position + glm::vec3(station->radius + 500.0f, 0.0f, 0.0f);
            auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
            if (playerEntity) {
                Health currentHealth = playerEntity->getHealth();
                m_gameClient->getEntityManager().updateEntityState(
                    m_localPlayerId, undockPos, glm::vec3(0.0f), 0.0f, currentHealth);
            }
        }
    }

    m_dockedStationId.clear();
    m_playerVelocity = glm::vec3(0.0f);
    m_playerSpeed = 0.0f;
    m_currentMoveCommand = MoveCommand::None;
    requestStateTransition(GameState::InSpace);
}

void Application::enterStationInterior() {
    if (m_gameState == GameState::Docked) {
        requestStateTransition(GameState::StationInterior);
        // Place camera at an interior spawn point
        m_camera->setFPSPosition(glm::vec3(0.0f, 1.8f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    }
}

void Application::enterShipInterior() {
    if (m_gameState == GameState::Docked || m_gameState == GameState::StationInterior) {
        requestStateTransition(GameState::ShipInterior);
        // Place camera inside the ship bridge area
        m_camera->setFPSPosition(glm::vec3(0.0f, 1.6f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    }
}

void Application::enterCockpit() {
    if (m_gameState == GameState::ShipInterior || m_gameState == GameState::Docked) {
        requestStateTransition(GameState::Cockpit);
        // Cockpit camera: slightly elevated, looking forward
        m_camera->setFPSPosition(glm::vec3(0.0f, 1.4f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    }
}

void Application::returnToHangar() {
    if (m_gameState == GameState::StationInterior ||
        m_gameState == GameState::ShipInterior ||
        m_gameState == GameState::Cockpit) {
        requestStateTransition(GameState::Docked);
    }
}

} // namespace atlas
