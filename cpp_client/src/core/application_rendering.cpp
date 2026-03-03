#include "core/application.h"
#include "rendering/window.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/solar_system_scene.h"
#include "ui/input_handler.h"
#include "ui/context_menu.h"
#include "ui/radial_menu.h"
#include "ui/atlas/atlas_context.h"
#include "ui/atlas/atlas_hud.h"
#include "ui/atlas/atlas_console.h"
#include "ui/atlas/atlas_pause_menu.h"
#include "ui/atlas/atlas_title_screen.h"
#ifdef NOVAFORGE_EDITOR_TOOLS
#include "editor/editor_tool_layer.h"
#endif

namespace atlas {

void Application::render() {
    // Clear screen
    m_renderer->clear(glm::vec4(0.01f, 0.01f, 0.05f, 1.0f));
    
    // Begin rendering
    m_renderer->beginFrame();
    
    // Update camera aspect ratio
    m_camera->setAspectRatio(m_window->getAspectRatio());
    m_camera->update(0.016f);
    
    // Update entity visuals from game client
    m_renderer->updateEntityVisuals(m_gameClient->getEntityManager().getAllEntities());
    
    // Render scene
    m_renderer->renderScene(*m_camera);
    
    // Render warp tunnel overlay (after 3D scene, before UI)
    if (m_solarSystem) {
        const auto& ws = m_solarSystem->getWarpVisualState();
        float intensity = ws.active ? 1.0f : 0.0f;
        m_renderer->updateWarpEffect(ws.phase, ws.progress, intensity,
                                      ws.direction,
                                      m_deltaTime);
        m_renderer->renderWarpEffect();
    }
    
    // Render Atlas HUD overlay
    {
        atlas::InputState atlasInput;
        atlasInput.windowW = m_window->getWidth();
        atlasInput.windowH = m_window->getHeight();
        // Forward mouse state from InputHandler to Atlas for interactive widgets
        atlasInput.mousePos = {static_cast<float>(m_inputHandler->getMouseX()),
                                static_cast<float>(m_inputHandler->getMouseY())};
        atlasInput.mouseDown[0] = m_inputHandler->isMouseDown(0);
        atlasInput.mouseDown[1] = m_inputHandler->isMouseDown(1);
        atlasInput.mouseDown[2] = m_inputHandler->isMouseDown(2);
        atlasInput.mouseClicked[0]  = m_inputHandler->isMouseClicked(0);
        atlasInput.mouseClicked[1]  = m_inputHandler->isMouseClicked(1);
        atlasInput.mouseClicked[2]  = m_inputHandler->isMouseClicked(2);
        atlasInput.mouseReleased[0] = m_inputHandler->isMouseReleased(0);
        atlasInput.mouseReleased[1] = m_inputHandler->isMouseReleased(1);
        atlasInput.mouseReleased[2] = m_inputHandler->isMouseReleased(2);
        atlasInput.scrollY = m_inputHandler->getScrollDeltaY();
        
        m_atlasCtx->beginFrame(atlasInput);

        // Title screen — replaces the entire HUD when active
        if (m_titleScreen && m_titleScreen->isActive()) {
            m_titleScreen->render(*m_atlasCtx);
            m_atlasCtx->endFrame();
            m_atlasConsumedMouse = m_atlasCtx->isMouseConsumed();
            m_renderer->endFrame();
            return;
        }
        
        atlas::ShipHUDData shipData;
        // Connect to actual ship state from game client
        auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
        if (playerEntity) {
            const auto& health = playerEntity->getHealth();
            shipData.shieldPct = health.maxShield > 0 ? health.currentShield / static_cast<float>(health.maxShield) : 0.0f;
            shipData.armorPct = health.maxArmor > 0 ? health.currentArmor / static_cast<float>(health.maxArmor) : 0.0f;
            shipData.hullPct = health.maxHull > 0 ? health.currentHull / static_cast<float>(health.maxHull) : 0.0f;
            const auto& capacitor = playerEntity->getCapacitor();
            shipData.capacitorPct = capacitor.max > 0.0f ? capacitor.current / capacitor.max : 0.0f;
        }
        shipData.currentSpeed = m_playerSpeed;
        shipData.maxSpeed = m_playerMaxSpeed;
        
        // Feed warp state into HUD
        if (m_solarSystem) {
            const auto& ws = m_solarSystem->getWarpVisualState();
            shipData.warpActive   = ws.active;
            shipData.warpPhase    = ws.phase;
            shipData.warpProgress = ws.progress;
            shipData.warpSpeedAU  = ws.speedAU;
        }
        
        // Build Atlas target cards from target list
        std::vector<atlas::TargetCardInfo> atlasTargets;
        if (playerEntity) {
            const auto playerPos = playerEntity->getPosition();
            for (const auto& targetId : m_targetList) {
                auto targetEntity = m_gameClient->getEntityManager().getEntity(targetId);
                if (!targetEntity) continue;
                atlas::TargetCardInfo card;
                card.name = targetEntity->getShipName().empty() ? targetEntity->getId() : targetEntity->getShipName();
                const auto& th = targetEntity->getHealth();
                card.shieldPct = th.maxShield > 0 ? th.currentShield / static_cast<float>(th.maxShield) : 0.0f;
                card.armorPct = th.maxArmor > 0 ? th.currentArmor / static_cast<float>(th.maxArmor) : 0.0f;
                card.hullPct = th.maxHull > 0 ? th.currentHull / static_cast<float>(th.maxHull) : 0.0f;
                card.distance = glm::distance(playerPos, targetEntity->getPosition());
                card.isActive = (targetId == m_currentTargetId);
                atlasTargets.push_back(card);
            }
        }
        
        // Build Atlas overview entries from entity manager
        std::vector<atlas::OverviewEntry> atlasOverview;
        if (playerEntity) {
            const auto playerPos = playerEntity->getPosition();
            for (const auto& pair : m_gameClient->getEntityManager().getAllEntities()) {
                if (pair.first == m_localPlayerId) continue;
                atlas::OverviewEntry entry;
                entry.entityId = pair.first;
                entry.name = pair.second->getShipName().empty() ? pair.first : pair.second->getShipName();
                entry.type = pair.second->getShipType();
                entry.distance = glm::distance(playerPos, pair.second->getPosition());
                entry.selected = (pair.first == m_currentTargetId);
                atlasOverview.push_back(entry);
            }
            
            // Add solar system celestials (planets, stations, gates, belts)
            if (m_solarSystem) {
                for (const auto& c : m_solarSystem->getCelestials()) {
                    if (c.type == atlas::Celestial::Type::SUN) continue;
                    atlas::OverviewEntry entry;
                    entry.entityId = c.id;
                    entry.name = c.name;
                    entry.distance = glm::distance(playerPos, c.position);
                    entry.selected = false;
                    switch (c.type) {
                        case atlas::Celestial::Type::PLANET:        entry.type = "Planet";        break;
                        case atlas::Celestial::Type::MOON:          entry.type = "Moon";          break;
                        case atlas::Celestial::Type::STATION:       entry.type = "Station";       break;
                        case atlas::Celestial::Type::STARGATE:      entry.type = "Stargate";      break;
                        case atlas::Celestial::Type::ASTEROID_BELT: entry.type = "Asteroid Belt"; break;
                        case atlas::Celestial::Type::WORMHOLE:      entry.type = "Wormhole";      break;
                        default:                                  entry.type = "Celestial";     break;
                    }
                    atlasOverview.push_back(entry);
                }
            }
        }
        
        // Build selected item info
        atlas::SelectedItemInfo atlasSelected;
        if (!m_currentTargetId.empty() && playerEntity) {
            auto targetEntity = m_gameClient->getEntityManager().getEntity(m_currentTargetId);
            if (targetEntity) {
                atlasSelected.name = targetEntity->getShipName().empty() ? m_currentTargetId : targetEntity->getShipName();
                float dist = glm::distance(playerEntity->getPosition(), targetEntity->getPosition());
                if (dist >= 1000.0f) {
                    atlasSelected.distance = dist / 1000.0f;
                    atlasSelected.distanceUnit = "km";
                } else {
                    atlasSelected.distance = dist;
                    atlasSelected.distanceUnit = "m";
                }
            }
        }
        
        // Update mode indicator text on the HUD
        m_atlasHUD->setModeIndicator(m_activeModeText);
        
        // Reserve context menu / radial menu input areas BEFORE panels
        // so their clicks aren't stolen by panel body consumption.
        if (m_contextMenu && m_contextMenu->IsOpen()) {
            m_contextMenu->ReserveInputArea(*m_atlasCtx);
        }

        // Render HUD panels (overview, selected item, ship HUD, etc.)
        m_atlasHUD->update(*m_atlasCtx, shipData, atlasTargets, atlasOverview, atlasSelected);

        // Render Context Menu AFTER panels so it draws on top visually
        if (m_contextMenu && m_contextMenu->IsOpen()) {
            m_contextMenu->RenderAtlas(*m_atlasCtx);
        }

        // Render Radial Menu on top of everything
        if (m_radialMenu && m_radialMenuOpen) {
            m_radialMenu->RenderAtlas(*m_atlasCtx);
        }

        // Render Pause Menu overlay (on top of game HUD)
        if (m_pauseMenu && m_pauseMenu->isOpen()) {
            m_pauseMenu->render(*m_atlasCtx);
        }

#ifdef NOVAFORGE_EDITOR_TOOLS
        // Render Editor Tool Layer overlay (F12 toggle)
        if (m_editorToolLayer && m_editorToolLayer->isActive()) {
            m_editorToolLayer->draw(*m_atlasCtx);
        }
#endif

        // Render Console overlay (topmost layer)
        if (m_console && m_console->isOpen()) {
            m_console->render(*m_atlasCtx);
        }
        
        m_atlasCtx->endFrame();

        // Record whether Atlas UI consumed the mouse this frame so that
        // game-world interaction handlers can avoid click-through
        m_atlasConsumedMouse = m_atlasCtx->isMouseConsumed();
    }
    
    // End rendering
    m_renderer->endFrame();
}

} // namespace atlas
