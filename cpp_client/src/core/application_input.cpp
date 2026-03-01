#include "core/application.h"
#include "rendering/window.h"
#include "rendering/camera.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/solar_system_scene.h"
#include "core/ship_physics.h"
#include "ui/input_handler.h"
#include "ui/entity_picker.h"
#include "ui/context_menu.h"
#include "ui/radial_menu.h"
#include "ui/atlas/atlas_hud.h"
#include "ui/atlas/atlas_console.h"
#include "ui/atlas/atlas_pause_menu.h"
#include "ui/atlas/atlas_title_screen.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace atlas {

void Application::handleKeyInput(int key, int action, int mods) {
    // Only handle PRESS events for most keys
    if (action != GLFW_PRESS) {
        return;
    }

    // Don't process game keys when console, pause menu, or title screen is active
    if ((m_console && m_console->wantsKeyboardInput()) ||
        (m_pauseMenu && m_pauseMenu->wantsKeyboardInput()) ||
        (m_titleScreen && m_titleScreen->wantsKeyboardInput())) {
        return;
    }

    // Module activation (F1-F8) — Astralis standard
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F8) {
        int slot = key - GLFW_KEY_F1 + 1;  // F1 = slot 1
        activateModule(slot);
        return;
    }
    
    // Tab — cycle targets (Astralis standard)
    if (key == GLFW_KEY_TAB) {
        cycleTarget();
        return;
    }
    
    // CTRL+SPACE — stop ship (Astralis standard)
    if (key == GLFW_KEY_SPACE && (mods & GLFW_MOD_CONTROL)) {
        commandStopShip();
        return;
    }
    
    // Astralis-style shortcut keys with modifier:
    // Q + click = Approach (we just toggle approach mode)
    // W + click = Orbit
    // E + click = Keep at Range
    // D + click = Dock/Jump Through
    if (key == GLFW_KEY_Q) {
        m_approachActive = true;
        m_orbitActive = false;
        m_keepRangeActive = false;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "APPROACH - click a target";
        std::cout << "[Controls] Approach mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_W) {
        m_approachActive = false;
        m_orbitActive = true;
        m_keepRangeActive = false;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "ORBIT - click a target";
        std::cout << "[Controls] Orbit mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_E) {
        m_approachActive = false;
        m_orbitActive = false;
        m_keepRangeActive = true;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "KEEP AT RANGE - click a target";
        std::cout << "[Controls] Keep at Range mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_D) {
        m_approachActive = false;
        m_orbitActive = false;
        m_keepRangeActive = false;
        m_dockingModeActive = true;
        m_warpModeActive = false;
        m_activeModeText = "DOCK / JUMP - click a station or gate";
        std::cout << "[Controls] Docking mode active — click a station or gate" << std::endl;
    } else if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL)) {
        // Ctrl+S = stop ship (Astralis standard) — checked before bare S
        commandStopShip();
    } else if (key == GLFW_KEY_S) {
        // S + Click = Warp To (Astralis standard)
        m_approachActive = false;
        m_orbitActive = false;
        m_keepRangeActive = false;
        m_dockingModeActive = false;
        m_warpModeActive = true;
        m_activeModeText = "WARP TO - click a target";
        std::cout << "[Controls] Warp mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_F) {
        // F = Engage/Recall drones (Astralis standard)
        std::cout << "[Controls] Drone command: engage/recall" << std::endl;
        auto* networkMgr = m_gameClient->getNetworkManager();
        if (networkMgr && networkMgr->isConnected()) {
            networkMgr->sendDroneCommand("engage_toggle", m_currentTargetId);
        }
    } else if (key == GLFW_KEY_V) {
        // V = Toggle view mode (Orbit ↔ Cockpit ↔ FPS depending on game state)
        toggleViewMode();
    }
    
    // Panel toggles (Atlas HUD)
    if (key == GLFW_KEY_O && (mods & GLFW_MOD_ALT)) {
        m_atlasHUD->toggleOverview();
    }
}

void Application::handleMouseButton(int button, int action, int mods, double x, double y) {
    // Track button state for camera control
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            m_rightMouseDown = true;
            m_lastMouseDragX = x;
            m_lastMouseDragY = y;
        } else if (action == GLFW_RELEASE) {
            // If right-click was a quick click (not a drag), show context menu
            // Skip if UI already captured the mouse (e.g. overview panel handled it)
            // to prevent two context menus appearing simultaneously
            if (m_rightMouseDown) {
                if (!m_atlasConsumedMouse) {
                    double dx = x - m_lastMouseDragX;
                    double dy = y - m_lastMouseDragY;
                    double dist = std::sqrt(dx * dx + dy * dy);
                    if (dist < 5.0) {
                        // Quick right-click — show context menu
                        // Pick entity at mouse position
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
                        
                        if (!pickedId.empty()) {
                            // Show entity context menu
                            bool isLocked = std::find(m_targetList.begin(), m_targetList.end(), pickedId) != m_targetList.end();
                            bool isStargate = false;
                            if (m_solarSystem) {
                                const auto* cel = m_solarSystem->findCelestial(pickedId);
                                if (cel && cel->type == atlas::Celestial::Type::STARGATE)
                                    isStargate = true;
                            }
                            m_contextMenu->ShowEntityMenu(pickedId, isLocked, isStargate);
                            m_contextMenu->SetScreenPosition(static_cast<float>(x), static_cast<float>(y));
                        } else {
                            // Show empty space context menu
                            m_contextMenu->ShowEmptySpaceMenu(0.0f, 0.0f, 0.0f);
                            m_contextMenu->SetScreenPosition(static_cast<float>(x), static_cast<float>(y));
                        }
                    }
                }
            }
            m_rightMouseDown = false;
        }
    }
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_leftMouseDown = true;
            m_radialMenuStartX = x;
            m_radialMenuStartY = y;
            m_radialMenuHoldStartTime = glfwGetTime();

            // Close context menu when clicking elsewhere (Astralis behaviour)
            if (m_contextMenu && m_contextMenu->IsOpen()) {
                m_contextMenu->Close();
            }
        } else if (action == GLFW_RELEASE) {
            // Check if radial menu is open
            if (m_radialMenuOpen) {
                // Confirm selection
                auto confirmedAction = m_radialMenu->Confirm();
                m_radialMenuOpen = false;
                m_radialMenu->Close();
            }
            m_leftMouseDown = false;
        }
    }
    
    // Left-click: select entity / apply movement command
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Don't process clicks that UI captured
        // Atlas UI consumption is the primary gate
        if (m_atlasConsumedMouse) return;
        
        // Pick entity at mouse position
        auto entities = m_gameClient->getEntityManager().getAllEntities();
        std::vector<std::shared_ptr<Entity>> entityList;
        for (const auto& pair : entities) {
            if (pair.first != m_localPlayerId) {  // Don't pick yourself
                entityList.push_back(pair.second);
            }
        }
        
        std::string pickedEntityId = m_entityPicker->pickEntity(
            x, y,
            m_window->getWidth(), m_window->getHeight(),
            *m_camera,
            entityList
        );
        
        if (!pickedEntityId.empty()) {
            // Astralis-style: Apply pending movement command if one is active
            if (m_approachActive) {
                commandApproach(pickedEntityId);
                m_approachActive = false;
                m_activeModeText.clear();
            } else if (m_orbitActive) {
                commandOrbit(pickedEntityId);
                m_orbitActive = false;
                m_activeModeText.clear();
            } else if (m_keepRangeActive) {
                commandKeepAtRange(pickedEntityId);
                m_keepRangeActive = false;
                m_activeModeText.clear();
            } else if (m_dockingModeActive) {
                // D+Click for docking/jump through
                std::cout << "[Movement] Dock/Jump through " << pickedEntityId << std::endl;
                // Check if it's a stargate → jump, otherwise try to dock
                if (m_solarSystem) {
                    const auto* cel = m_solarSystem->findCelestial(pickedEntityId);
                    if (cel && cel->type == atlas::Celestial::Type::STARGATE) {
                        commandJump(pickedEntityId);
                    } else if (cel && cel->type == atlas::Celestial::Type::STATION) {
                        requestDock();
                    }
                }
                m_dockingModeActive = false;
                m_activeModeText.clear();
            } else if (m_warpModeActive) {
                // S+Click for warp to (Astralis standard)
                commandWarpTo(pickedEntityId);
                m_warpModeActive = false;
                m_activeModeText.clear();
            } else {
                // Default: select / CTRL+click to lock target / double-click to approach
                if (m_inputHandler->isDoubleClick()) {
                    // Astralis-style double-click: approach the entity
                    commandApproach(pickedEntityId);
                } else {
                    bool addToTargets = (mods & GLFW_MOD_CONTROL) != 0;
                    targetEntity(pickedEntityId, addToTargets);
                }
            }
        }
    }
}

void Application::handleMouseMove(double x, double y, double deltaX, double deltaY) {
    // Update radial menu if open
    if (m_radialMenuOpen && m_radialMenu) {
        m_radialMenu->UpdateMousePosition(static_cast<float>(x), static_cast<float>(y));
    }
    
    // Check if we should open radial menu (left mouse held for RADIAL_MENU_HOLD_TIME)
    if (m_leftMouseDown && !m_radialMenuOpen) {
        double holdTime = glfwGetTime() - m_radialMenuHoldStartTime;
        if (holdTime >= RADIAL_MENU_HOLD_TIME) {
            // Check distance moved
            double dx = x - m_radialMenuStartX;
            double dy = y - m_radialMenuStartY;
            double dist = std::sqrt(dx * dx + dy * dy);
            
            // Only open if not dragging significantly
            if (dist < 10.0) {
                // ── FPS mode: open context-aware FPS radial menu ───────
                if (m_gameState == GameState::StationInterior ||
                    m_gameState == GameState::ShipInterior) {
                    // In FPS mode, open the radial menu in the screen center
                    // with context based on what the player is looking at
                    float cx = static_cast<float>(m_window->getWidth()) * 0.5f;
                    float cy = static_cast<float>(m_window->getHeight()) * 0.5f;

                    // Pick the nearest interactable entity from the crosshair
                    auto entities = m_gameClient->getEntityManager().getAllEntities();
                    std::string nearestId;
                    float nearestDist = 4.0f; // 4m max FPS interaction range
                    UI::RadialMenu::InteractionContext fpsCtx =
                        UI::RadialMenu::InteractionContext::None;
                    std::string fpsDisplayName;
                    bool fpsIsDoorOpen = false;
                    bool fpsIsLocked = false;

                    for (const auto& pair : entities) {
                        const auto& ent = pair.second;
                        if (!ent) continue;
                        // Check tag for interactable type
                        const std::string& tag = ent->getTag();
                        if (tag.empty()) continue;

                        // Determine distance (simplified — use entity distance)
                        float d = glm::distance(m_camera->getPosition(), ent->getPosition());
                        if (d < nearestDist) {
                            UI::RadialMenu::InteractionContext ctx =
                                UI::RadialMenu::InteractionContext::None;
                            bool doorOpen = false;
                            bool locked = false;

                            if (tag == "door") {
                                ctx = UI::RadialMenu::InteractionContext::Door;
                            } else if (tag == "security_door") {
                                ctx = UI::RadialMenu::InteractionContext::SecurityDoor;
                                locked = true; // Default hint, actual state from server
                            } else if (tag == "airlock") {
                                ctx = UI::RadialMenu::InteractionContext::Airlock;
                            } else if (tag == "terminal") {
                                ctx = UI::RadialMenu::InteractionContext::Terminal;
                            } else if (tag == "loot_container") {
                                ctx = UI::RadialMenu::InteractionContext::LootContainer;
                            } else if (tag == "fabricator") {
                                ctx = UI::RadialMenu::InteractionContext::Fabricator;
                            } else if (tag == "medical_bay") {
                                ctx = UI::RadialMenu::InteractionContext::MedicalBay;
                            }

                            if (ctx != UI::RadialMenu::InteractionContext::None) {
                                nearestDist = d;
                                nearestId = pair.first;
                                fpsCtx = ctx;
                                fpsDisplayName = ent->getName().empty() ? tag : ent->getName();
                                fpsIsDoorOpen = doorOpen;
                                fpsIsLocked = locked;
                            }
                        }
                    }

                    if (!nearestId.empty()) {
                        m_radialMenu->OpenFPS(cx, cy, nearestId, fpsCtx,
                                              fpsDisplayName, fpsIsDoorOpen, fpsIsLocked);
                        m_radialMenuOpen = true;
                        std::cout << "[Radial Menu] FPS mode opened for: " << fpsDisplayName
                                  << " (" << nearestId << ")" << std::endl;
                    }
                }
                // ── Space mode: open standard space radial menu ────────
                else {
                // Pick entity at hold position
                auto entities = m_gameClient->getEntityManager().getAllEntities();
                std::vector<std::shared_ptr<Entity>> entityList;
                for (const auto& pair : entities) {
                    if (pair.first != m_localPlayerId) {
                        entityList.push_back(pair.second);
                    }
                }
                
                std::string pickedId = m_entityPicker->pickEntity(
                    m_radialMenuStartX, m_radialMenuStartY,
                    m_window->getWidth(), m_window->getHeight(),
                    *m_camera, entityList);
                
                if (!pickedId.empty()) {
                    // Compute distance to target for warp eligibility check
                    float distToTarget = 0.0f;
                    if (m_shipPhysics) {
                        auto targetEntity = m_gameClient->getEntityManager().getEntity(pickedId);
                        if (targetEntity) {
                            distToTarget = glm::distance(m_shipPhysics->getPosition(), targetEntity->getPosition());
                        }
                    }
                    m_radialMenu->Open(static_cast<float>(m_radialMenuStartX), 
                                      static_cast<float>(m_radialMenuStartY), 
                                      pickedId, distToTarget);
                    m_radialMenuOpen = true;
                    std::cout << "[Radial Menu] Opened for entity: " << pickedId
                              << " (distance: " << (distToTarget / 1000.0f) << " km)" << std::endl;
                }
                } // end space mode
            }
        }
    }
    
    // Astralis-style camera: Right-click drag to orbit camera around ship
    // In FPS/Cockpit mode, right-drag does mouse-look instead
    if (m_rightMouseDown) {
        if (!m_atlasConsumedMouse) {
            float sensitivity = 0.3f;
            if (m_camera->getViewMode() == atlas::ViewMode::FPS ||
                m_camera->getViewMode() == atlas::ViewMode::COCKPIT) {
                m_camera->rotateFPS(static_cast<float>(deltaX) * sensitivity,
                                    static_cast<float>(-deltaY) * sensitivity);
            } else {
                m_camera->rotate(static_cast<float>(deltaX) * sensitivity,
                               static_cast<float>(-deltaY) * sensitivity);
            }
        }
    }
}

void Application::handleScroll(double xoffset, double yoffset) {
    // Astralis-style: mousewheel zooms camera
    if (!m_atlasConsumedMouse) {
        m_camera->zoom(static_cast<float>(yoffset));
    }
}

} // namespace atlas
