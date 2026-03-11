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

    // ── FPS Mode Controls ─────────────────────────────────────────────
    // When on foot (Docked, StationInterior, ShipInterior), WASD/Space/etc.
    // drive first-person movement instead of tactical fleet commands.
    // ESC is handled upstream in the window key callback (releases cursor +
    // toggles pause menu) so we skip it here to avoid double-handling.
    if (isInFPSMode()) {
        if (key != GLFW_KEY_ESCAPE) {
            handleFPSKeyInput(key, action, mods);
        }
        return;
    }

    // ── Tactical / Space Mode Controls ────────────────────────────────
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
    
    // Tactical shortcut keys for navigation commands.
    // WASD are reserved for direct ship flight (see updateLocalMovement).
    // Q and E trigger mode-then-click auto-navigation.
    // Dock/Jump and Warp-to are accessible via right-click context menu.
    if (key == GLFW_KEY_Q) {
        // Q = Approach — set approach mode then click a target
        m_approachActive = true;
        m_orbitActive = false;
        m_keepRangeActive = false;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "APPROACH - click a target";
        std::cout << "[Controls] Approach mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_E) {
        m_approachActive = false;
        m_orbitActive = false;
        m_keepRangeActive = true;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "KEEP AT RANGE - click a target";
        std::cout << "[Controls] Keep at Range mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL)) {
        // Ctrl+S = stop ship
        commandStopShip();
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

void Application::handleFPSKeyInput(int key, int /*action*/, int /*mods*/) {
    // FPS-mode key press events (one-shot actions)
    // Continuous WASD movement is handled in updateFPSMovement() via key polling.
    switch (key) {
        case GLFW_KEY_SPACE:
            // Jump — request one jump per press
            std::cout << "[FPS] Jump" << std::endl;
            m_fpsJumpRequested = true;
            break;

        case GLFW_KEY_E:
            // Interact with nearest object
            std::cout << "[FPS] Interact" << std::endl;
            // TODO: dispatch interaction to nearest interactable entity
            break;

        case GLFW_KEY_V:
            // Toggle view mode (FPS ↔ Cockpit if in ship)
            toggleViewMode();
            break;

        case GLFW_KEY_R:
            // Reload / context action
            std::cout << "[FPS] Reload" << std::endl;
            break;

        case GLFW_KEY_F:
            // Toggle flashlight
            std::cout << "[FPS] Toggle flashlight" << std::endl;
            m_fpsFlashlightOn = !m_fpsFlashlightOn;
            break;

        case GLFW_KEY_TAB:
            // Open inventory
            std::cout << "[FPS] Inventory toggle" << std::endl;
            break;

        case GLFW_KEY_F1:
            // FPS quick-slot 1 (re-purposing F1-F4 for FPS item slots)
            std::cout << "[FPS] Quick slot 1" << std::endl;
            break;

        case GLFW_KEY_F2:
            std::cout << "[FPS] Quick slot 2" << std::endl;
            break;

        case GLFW_KEY_F3:
            std::cout << "[FPS] Quick slot 3" << std::endl;
            break;

        case GLFW_KEY_F4:
            std::cout << "[FPS] Quick slot 4" << std::endl;
            break;

        default:
            break;
    }
}

void Application::handleMouseButton(int button, int action, int mods, double x, double y) {
    // ── FPS Mode: mouse buttons have different behaviour ──────────────
    if (isInFPSMode()) {
        // In FPS mode, clicking re-captures cursor if it was released (e.g. after Escape)
        if (!m_fpsCursorCaptured && action == GLFW_PRESS) {
            captureFPSCursor();
            return;  // Don't process this click as gameplay input
        }

        // Left-click = primary action (interact / use / fire)
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            std::cout << "[FPS] Primary action" << std::endl;
        }
        // Right-click = secondary action (aim / alt-fire / block)
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            std::cout << "[FPS] Secondary action" << std::endl;
        }
        return;  // Don't fall through to tactical handlers
    }

    // ── Tactical / Space Mode: original Astralis-style handling ───────
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
    // ── FPS Mode: free mouse look (no button hold required) ───────────
    if (isInFPSMode() && m_fpsCursorCaptured) {
        float sensitivity = 0.15f;
        m_fpsYaw   += static_cast<float>(deltaX) * sensitivity;
        m_fpsPitch -= static_cast<float>(deltaY) * sensitivity;

        // Clamp pitch to prevent flipping
        if (m_fpsPitch >  89.0f) m_fpsPitch =  89.0f;
        if (m_fpsPitch < -89.0f) m_fpsPitch = -89.0f;

        // Wrap yaw
        if (m_fpsYaw >  360.0f) m_fpsYaw -= 360.0f;
        if (m_fpsYaw < -360.0f) m_fpsYaw += 360.0f;

        // Apply to camera — rotateFPS updates the camera's internal yaw/pitch
        // and recomputes m_fpsForward.  The Application m_fpsYaw/m_fpsPitch
        // variables above are kept in sync so that movement code uses the
        // same angles as the visual camera direction.
        m_camera->rotateFPS(static_cast<float>(deltaX) * sensitivity,
                            static_cast<float>(-deltaY) * sensitivity);
        return;  // Don't fall through to tactical camera
    }

    // ── Tactical / Space Mode: original handling ─────────────────────
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
            if (dist < MAX_DRAG_THRESHOLD_PX) {
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
                    float nearestDist = MAX_FPS_INTERACTION_RANGE;
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

// ═══════════════════════════════════════════════════════════════════════
//  FPS Mode Helpers
// ═══════════════════════════════════════════════════════════════════════

bool Application::isInFPSMode() const {
    return m_gameState == GameState::Docked ||
           m_gameState == GameState::StationInterior ||
           m_gameState == GameState::ShipInterior;
}

bool Application::isInFlightMode() const {
    // ShipInterior uses 6DOF free-flight physics (no gravity).
    return m_gameState == GameState::ShipInterior;
}

void Application::captureFPSCursor() {
    if (m_fpsCursorCaptured) return;
    if (m_window && m_window->getHandle()) {
        glfwSetInputMode(m_window->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_fpsCursorCaptured = true;
        std::cout << "[FPS] Cursor captured" << std::endl;
    }
}

void Application::releaseFPSCursor() {
    if (!m_fpsCursorCaptured) return;
    if (m_window && m_window->getHandle()) {
        glfwSetInputMode(m_window->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_fpsCursorCaptured = false;
        std::cout << "[FPS] Cursor released" << std::endl;
    }
}

// ── Dispatcher ────────────────────────────────────────────────────────

void Application::updateFPSMovement(float deltaTime) {
    if (isInFlightMode()) {
        updateFlightMovement(deltaTime);
    } else {
        updateOnFootMovement(deltaTime);
    }
}

// ── On-foot movement (Docked / StationInterior) ───────────────────────
//
// WASD strafes / walks on the XZ plane relative to the player's look
// direction.  Gravity is applied every tick; jumping is allowed when
// grounded.  The camera stays at eye height above the floor.

void Application::updateOnFootMovement(float deltaTime) {
    if (!m_window || !m_window->getHandle()) return;

    GLFWwindow* win = m_window->getHandle();

    // ── Poll WASD ─────────────────────────────────────────────────────
    float moveX = 0.0f;   // strafe: negative = left, positive = right
    float moveZ = 0.0f;   // forward: positive = forward (into the scene)

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) moveZ += 1.0f;
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) moveZ -= 1.0f;
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) moveX -= 1.0f;
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) moveX += 1.0f;

    // ── Stance ────────────────────────────────────────────────────────
    bool sprintHeld = (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    bool crouchHeld = (glfwGetKey(win, GLFW_KEY_C)          == GLFW_PRESS) ||
                      (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL)== GLFW_PRESS);

    float speed = FPS_WALK_SPEED;
    if (sprintHeld && !crouchHeld && (moveX != 0.0f || moveZ != 0.0f)) {
        speed = FPS_SPRINT_SPEED;
        m_activeModeText = "SPRINTING";
    } else if (crouchHeld) {
        speed = FPS_CROUCH_SPEED;
        m_activeModeText = "CROUCHING";
    } else if (moveX != 0.0f || moveZ != 0.0f) {
        m_activeModeText = "WALKING";
    } else {
        m_activeModeText.clear();
    }

    // ── Standard FPS movement template ───────────────────────────────
    // Derive forward and right directly from the live camera orientation.
    // This is more robust than recomputing from a yaw angle and guarantees
    // movement is always exactly relative to what the player sees on screen.
    glm::vec3 camFwd  = m_camera->getFPSForward();
    // Project forward onto the horizontal plane so looking up/down does not
    // affect XZ speed (standard FPS behaviour).
    glm::vec3 forward = glm::vec3(camFwd.x, 0.0f, camFwd.z);
    if (glm::length(forward) > 0.001f)
        forward = glm::normalize(forward);
    else
        forward = glm::vec3(0.0f, 0.0f, -1.0f);  // fallback: looking straight up/down
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 moveDir = forward * moveZ + right * moveX;
    if (glm::length(moveDir) > 0.01f) {
        moveDir = glm::normalize(moveDir);
    }

    glm::vec3 velocity = moveDir * speed;

    // ── Jump ──────────────────────────────────────────────────────────
    if (m_fpsJumpRequested && m_fpsGrounded) {
        m_fpsVelY    = FPS_JUMP_IMPULSE;
        m_fpsGrounded = false;
    }
    m_fpsJumpRequested = false;

    // ── Gravity ───────────────────────────────────────────────────────
    if (!m_fpsGrounded) {
        m_fpsVelY -= FPS_GRAVITY * deltaTime;
    }

    // ── Apply position ────────────────────────────────────────────────
    glm::vec3 camPos = m_camera->getPosition();
    camPos += velocity * deltaTime;
    camPos.y += m_fpsVelY * deltaTime;

    // Floor collision — keep camera at the correct eye height for the
    // current stance.  When grounded the camera must track the target
    // eye height so that crouching/standing produces visible movement.
    float eyeHeight = crouchHeld ? FPS_CROUCH_EYE_HEIGHT : FPS_STAND_EYE_HEIGHT;
    if (camPos.y < eyeHeight) {
        camPos.y    = eyeHeight;
        m_fpsVelY   = 0.0f;
        m_fpsGrounded = true;
    }
    // When grounded (not jumping/falling), snap to the stance eye height
    // so that crouching lowers the camera and standing raises it.
    if (m_fpsGrounded) {
        camPos.y = eyeHeight;
    }

    m_camera->setFPSPosition(camPos, m_camera->getFPSForward());
}

// ── 6DOF flight movement (ShipInterior) ──────────────────────────────
//
// Inside a ship there is no gravity.  The player flies freely in all
// six degrees of freedom.  Mouse look steers the view; WASD moves
// forward/backward/strafe in the view plane; Q/E (or Space/Ctrl) move
// directly up/down in world space; Shift boosts speed.

void Application::updateFlightMovement(float deltaTime) {
    if (!m_window || !m_window->getHandle()) return;

    GLFWwindow* win = m_window->getHandle();

    // ── Poll directional keys ─────────────────────────────────────────
    float moveX = 0.0f;   // strafe
    float moveZ = 0.0f;   // forward/back
    float moveY = 0.0f;   // up/down (world Y)

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) moveZ += 1.0f;
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) moveZ -= 1.0f;
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) moveX -= 1.0f;
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) moveX += 1.0f;

    // Q / Space = ascend;  E / Left-Ctrl = descend
    if (glfwGetKey(win, GLFW_KEY_Q)            == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_SPACE)         == GLFW_PRESS) moveY += 1.0f;
    if (glfwGetKey(win, GLFW_KEY_E)            == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS) moveY -= 1.0f;

    bool boostHeld = (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    float speed = boostHeld ? FLIGHT_BOOST_SPEED : FLIGHT_NORMAL_SPEED;

    if (moveX != 0.0f || moveZ != 0.0f || moveY != 0.0f) {
        m_activeModeText = boostHeld ? "BOOST" : "FLYING";
    } else {
        m_activeModeText.clear();
    }

    // ── Standard 6DOF flight template ────────────────────────────────
    // Derive all direction vectors from the live camera so movement is
    // always exactly what the player sees, with no angle-sync drift.
    glm::vec3 camForward = m_camera->getFPSForward();
    // Horizontal right vector: cross(forward_xz, worldUp).
    // No Y tilt so strafing feels natural (standard FPS/EVA convention).
    glm::vec3 fwdXZ = glm::vec3(camForward.x, 0.0f, camForward.z);
    if (glm::length(fwdXZ) < 0.001f)
        fwdXZ = glm::vec3(0.0f, 0.0f, -1.0f);  // fallback: looking straight up/down
    else
        fwdXZ = glm::normalize(fwdXZ);
    glm::vec3 right = glm::normalize(glm::cross(fwdXZ, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::vec3 moveDir = camForward * moveZ + right * moveX + up * moveY;
    if (glm::length(moveDir) > 0.01f) {
        moveDir = glm::normalize(moveDir);
    }

    glm::vec3 velocity = moveDir * speed;

    // ── No gravity — reset vertical velocity ─────────────────────────
    m_fpsVelY     = 0.0f;
    m_fpsGrounded = false;
    m_fpsJumpRequested = false;

    // ── Apply position ────────────────────────────────────────────────
    glm::vec3 camPos = m_camera->getPosition();
    camPos += velocity * deltaTime;

    m_camera->setFPSPosition(camPos, m_camera->getFPSForward());
}

} // namespace atlas
