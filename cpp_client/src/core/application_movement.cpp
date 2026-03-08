#include "core/application.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/solar_system_scene.h"
#include "core/ship_physics.h"
#include "rendering/renderer.h"
#include "ui/atlas/atlas_hud.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace atlas {

void Application::commandApproach(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::Approach;
    m_moveTargetId = entityId;
    m_activeModeText = "APPROACHING";
    std::cout << "[Movement] Approaching " << entityId << std::endl;
}

void Application::commandOrbit(const std::string& entityId, float distance) {
    m_currentMoveCommand = MoveCommand::Orbit;
    m_moveTargetId = entityId;
    m_orbitDistance = distance;
    m_activeModeText = "ORBITING";
    std::cout << "[Movement] Orbiting " << entityId << " at " << distance << "m" << std::endl;
}

void Application::commandKeepAtRange(const std::string& entityId, float distance) {
    m_currentMoveCommand = MoveCommand::KeepAtRange;
    m_moveTargetId = entityId;
    m_keepAtRangeDistance = distance;
    m_activeModeText = "KEEP AT RANGE";
    std::cout << "[Movement] Keeping at range " << distance << "m from " << entityId << std::endl;
}

void Application::commandAlignTo(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::AlignTo;
    m_moveTargetId = entityId;
    m_activeModeText = "ALIGNING";
    std::cout << "[Movement] Aligning to " << entityId << std::endl;
}

void Application::commandWarpTo(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::WarpTo;
    m_moveTargetId = entityId;
    m_activeModeText = "WARPING";
    std::cout << "[Movement] Warping to " << entityId << std::endl;

    // Use ShipPhysics + SolarSystemScene for proper 4-phase warp
    if (m_solarSystem && m_shipPhysics) {
        // Try to warp via celestial lookup first
        const Celestial* celestial = m_solarSystem->findCelestial(entityId);
        if (celestial) {
            m_solarSystem->warpTo(entityId, m_shipPhysics.get());
            return;
        }
        // Fallback: warp to an entity position
        auto targetEntity = m_gameClient->getEntityManager().getEntity(entityId);
        if (targetEntity) {
            m_shipPhysics->warpTo(targetEntity->getPosition());
        }
    }
}

void Application::commandStopShip() {
    m_currentMoveCommand = MoveCommand::None;
    m_moveTargetId.clear();
    m_playerVelocity = glm::vec3(0.0f);
    m_playerSpeed = 0.0f;
    m_approachActive = false;
    m_orbitActive = false;
    m_keepRangeActive = false;
    m_dockingModeActive = false;
    m_activeModeText.clear();
    std::cout << "[Movement] Ship stopped" << std::endl;
}

void Application::commandJump(const std::string& entityId) {
    if (!m_solarSystem) return;

    const auto* gate = m_solarSystem->findCelestial(entityId);
    if (!gate || gate->type != atlas::Celestial::Type::STARGATE) {
        std::cout << "[Jump] " << entityId << " is not a stargate" << std::endl;
        return;
    }

    std::string destination = gate->linkedSystem;
    if (destination.empty()) {
        std::cout << "[Jump] Stargate " << entityId << " has no linked system" << std::endl;
        return;
    }

    std::cout << "[Jump] Jumping through stargate " << entityId
              << " to system: " << destination << std::endl;

    // Generate the destination system from its name hash
    uint32_t destSeed = 0;
    for (char c : destination) {
        destSeed = destSeed * 31 + static_cast<uint32_t>(c);
    }
    m_solarSystem->generateSystem(destSeed, destination);

    // Update sun rendering for the new system
    const auto* newSun = m_solarSystem->getSun();
    if (newSun) {
        m_renderer->setSunState(newSun->position, newSun->lightColor, newSun->radius);
    }

    // Update HUD system info
    if (m_atlasHUD) {
        m_atlasHUD->setSystemInfo(m_solarSystem->getSystemName(),
                                  m_solarSystem->getSecurityLevel());
    }

    // Reset player position to the arrival gate location
    const auto& celestials = m_solarSystem->getCelestials();
    glm::vec3 arrivalPos(0.0f);
    for (const auto& c : celestials) {
        if (c.type == atlas::Celestial::Type::STARGATE) {
            arrivalPos = c.position;
            break;  // arrive at the first gate in the new system
        }
    }

    glm::vec3 playerPos = arrivalPos + glm::vec3(2000.0f, 0.0f, 0.0f);  // offset from gate
    m_playerVelocity = glm::vec3(0.0f);
    m_playerSpeed = 0.0f;
    m_currentMoveCommand = MoveCommand::None;
    m_activeModeText.clear();

    // Update the player entity position through the entity manager
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (playerEntity) {
        float rotation = playerEntity->getRotation();
        Health currentHealth = playerEntity->getHealth();
        m_gameClient->getEntityManager().updateEntityState(
            m_localPlayerId, playerPos, m_playerVelocity, rotation, currentHealth);
    }

    std::cout << "[Jump] Arrived in " << destination << " at position ("
              << playerPos.x << ", " << playerPos.y << ", "
              << playerPos.z << ")" << std::endl;
}

void Application::updateLocalMovement(float deltaTime) {
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (!playerEntity) return;
    
    // Movement physics constants — tuned for Astralis-style feel with proper align time
    static constexpr float ACCELERATION = 25.0f;           // m/s² (reduced for gradual ramp-up)
    static constexpr float DECELERATION = 30.0f;            // m/s² when stopping (faster than accel for responsive stop)
    static constexpr float APPROACH_DECEL_DIST = 50.0f;     // Start slowing at this range
    static constexpr float WARP_SPEED = 5000.0f;            // Simulated warp speed m/s
    static constexpr float WARP_EXIT_DIST = 100.0f;         // Exit warp at this range
    static constexpr float ALIGN_TURN_RATE = 1.5f;          // rad/s — how fast ship rotates toward target
    static constexpr float ALIGN_SPEED_FRACTION = 0.75f;    // Must reach 75% max speed to warp
    
    glm::vec3 playerPos = playerEntity->getPosition();
    
    if (m_currentMoveCommand == MoveCommand::None) {
        // Decelerate to stop — exponential slowdown for smooth feel
        if (m_playerSpeed > 0.1f) {
            // Guard against negative factor when deltaTime is very large (e.g. lag spike)
            m_playerSpeed *= std::max(0.0f, 1.0f - DECELERATION * deltaTime / m_playerMaxSpeed);
            playerPos += m_playerVelocity * deltaTime;
            // Update velocity direction with reduced speed
            if (glm::length(m_playerVelocity) > 0.01f) {
                m_playerVelocity = glm::normalize(m_playerVelocity) * m_playerSpeed;
            }
        } else {
            m_playerSpeed = 0.0f;
            m_playerVelocity = glm::vec3(0.0f);
        }
    } else {
        // Get target position
        auto targetEntity = m_gameClient->getEntityManager().getEntity(m_moveTargetId);
        if (!targetEntity) {
            m_currentMoveCommand = MoveCommand::None;
            return;
        }
        
        glm::vec3 targetPos = targetEntity->getPosition();
        glm::vec3 toTarget = targetPos - playerPos;
        float dist = glm::length(toTarget);
        
        if (dist < 0.01f) return;  // Already at target
        
        glm::vec3 dir = glm::normalize(toTarget);
        
        switch (m_currentMoveCommand) {
            case MoveCommand::Approach: {
                // Astralis-style exponential acceleration towards target
                // Ship gradually ramps up speed, giving time to align
                float targetSpeed = m_playerMaxSpeed;
                if (dist < APPROACH_DECEL_DIST) {
                    targetSpeed = m_playerMaxSpeed * (dist / APPROACH_DECEL_DIST);
                }
                // Exponential ramp: speed approaches target over time
                float speedDiff = targetSpeed - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::clamp(m_playerSpeed, 0.0f, targetSpeed);
                m_playerVelocity = dir * m_playerSpeed;
                break;
            }
            case MoveCommand::Orbit: {
                // Orbit around target at set distance with gradual acceleration
                float speedDiff = m_playerMaxSpeed - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::min(m_playerSpeed, m_playerMaxSpeed);
                if (dist > m_orbitDistance + 10.0f) {
                    m_playerVelocity = dir * m_playerSpeed;
                } else if (dist < m_orbitDistance - 10.0f) {
                    m_playerVelocity = -dir * m_playerSpeed * 0.5f;
                } else {
                    // Orbit tangent
                    glm::vec3 tangent(-dir.z, 0.0f, dir.x);
                    m_playerVelocity = tangent * m_playerSpeed;
                }
                break;
            }
            case MoveCommand::KeepAtRange: {
                float speedDiff = m_playerMaxSpeed - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::min(m_playerSpeed, m_playerMaxSpeed);
                if (dist > m_keepAtRangeDistance + 20.0f) {
                    m_playerVelocity = dir * m_playerSpeed;
                } else if (dist < m_keepAtRangeDistance - 20.0f) {
                    m_playerVelocity = -dir * m_playerSpeed * 0.3f;
                } else {
                    m_playerSpeed = std::max(0.0f, m_playerSpeed - DECELERATION * deltaTime);
                    m_playerVelocity = dir * m_playerSpeed;
                }
                break;
            }
            case MoveCommand::AlignTo: {
                // Align to target: gradually accelerate to 75% max speed
                // giving the ship time to turn and align before reaching speed
                float alignTarget = m_playerMaxSpeed * ALIGN_SPEED_FRACTION;
                float speedDiff = alignTarget - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::clamp(m_playerSpeed, 0.0f, alignTarget);
                m_playerVelocity = dir * m_playerSpeed;
                break;
            }
            case MoveCommand::WarpTo: {
                // Use ShipPhysics 4-phase warp when available
                if (m_shipPhysics && m_shipPhysics->isWarping()) {
                    m_shipPhysics->update(deltaTime);
                    playerPos = m_shipPhysics->getPosition();
                    m_playerVelocity = m_shipPhysics->getVelocity();
                    m_playerSpeed = m_shipPhysics->getCurrentSpeed();

                    // Update mode text with warp phase info
                    auto phase = m_shipPhysics->getWarpPhase();
                    float speedAU = m_shipPhysics->getWarpSpeedAU();
                    switch (phase) {
                        case ShipPhysics::WarpPhase::ALIGNING:
                            m_activeModeText = "ALIGNING";
                            break;
                        case ShipPhysics::WarpPhase::ACCELERATING: {
                            char buf[64];
                            std::snprintf(buf, sizeof(buf), "WARP  %.1f AU/s", speedAU);
                            m_activeModeText = buf;
                            break;
                        }
                        case ShipPhysics::WarpPhase::CRUISING: {
                            char buf[64];
                            std::snprintf(buf, sizeof(buf), "WARP  %.1f AU/s", speedAU);
                            m_activeModeText = buf;
                            break;
                        }
                        case ShipPhysics::WarpPhase::DECELERATING:
                            m_activeModeText = "DECELERATING";
                            break;
                        default:
                            break;
                    }

                    // Warp completed?
                    if (!m_shipPhysics->isWarping()) {
                        m_currentMoveCommand = MoveCommand::None;
                        m_playerSpeed = m_shipPhysics->getCurrentSpeed();
                        m_playerVelocity = m_shipPhysics->getVelocity();
                        m_activeModeText.clear();
                        std::cout << "[Movement] Warp complete" << std::endl;
                    }
                    // Update entity via EntityManager (ShipPhysics owns position)
                    float rotation = 0.0f;
                    if (glm::length(m_playerVelocity) > 0.1f) {
                        rotation = std::atan2(m_playerVelocity.x, m_playerVelocity.z);
                    }
                    Health currentHealth = playerEntity->getHealth();
                    m_gameClient->getEntityManager().updateEntityState(
                        m_localPlayerId, playerPos, m_playerVelocity, rotation, currentHealth);
                    return;
                }
                // Fallback: simple linear warp (legacy path)
                m_playerSpeed = std::min(WARP_SPEED,
                                         m_playerSpeed + WARP_SPEED * deltaTime);
                m_playerVelocity = dir * m_playerSpeed;
                if (dist < WARP_EXIT_DIST) {
                    m_currentMoveCommand = MoveCommand::None;
                    m_playerSpeed = 0.0f;
                    m_playerVelocity = glm::vec3(0.0f);
                    m_activeModeText.clear();
                    std::cout << "[Movement] Warp complete" << std::endl;
                }
                break;
            }
            default:
                break;
        }
        
        playerPos += m_playerVelocity * deltaTime;
    }
    
    // Update player entity position
    float rotation = 0.0f;
    if (glm::length(m_playerVelocity) > 0.1f) {
        rotation = std::atan2(m_playerVelocity.x, m_playerVelocity.z);
    }
    
    Health currentHealth = playerEntity->getHealth();
    m_gameClient->getEntityManager().updateEntityState(
        m_localPlayerId, playerPos, m_playerVelocity, rotation, currentHealth);
}

} // namespace atlas
