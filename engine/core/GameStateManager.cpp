#include "GameStateManager.h"
#include <algorithm>

namespace atlas {

const char* GamePhaseName(GamePhase phase) {
    switch (phase) {
        case GamePhase::MainMenu: return "MainMenu";
        case GamePhase::Loading:  return "Loading";
        case GamePhase::InSpace:  return "InSpace";
        case GamePhase::Docked:   return "Docked";
        case GamePhase::InWarp:   return "InWarp";
        case GamePhase::Paused:   return "Paused";
    }
    return "Unknown";
}

void GameStateManager::SetPhase(GamePhase phase) {
    if (phase == m_phase) return;
    GamePhase old = m_phase;
    m_phase = phase;
    for (auto& cb : m_phaseCallbacks) {
        if (cb) cb(old, m_phase);
    }
}

void GameStateManager::OnPhaseChange(PhaseCallback cb) {
    if (cb) m_phaseCallbacks.push_back(std::move(cb));
}

bool GameStateManager::Spend(int64_t cost) {
    if (m_credits < cost) return false;
    m_credits -= cost;
    return true;
}

void GameStateManager::AddItem(uint32_t itemId) {
    m_inventory.push_back(itemId);
}

bool GameStateManager::RemoveItem(uint32_t itemId) {
    auto it = std::find(m_inventory.begin(), m_inventory.end(), itemId);
    if (it == m_inventory.end()) return false;
    m_inventory.erase(it);
    return true;
}

bool GameStateManager::HasItem(uint32_t itemId) const {
    return std::find(m_inventory.begin(), m_inventory.end(), itemId) != m_inventory.end();
}

void GameStateManager::Reset() {
    m_phase = GamePhase::MainMenu;
    m_playerEntity = 0;
    m_currentSystem = 0;
    m_dockedStation = 0;
    m_credits = 0;
    m_inventory.clear();
    // Note: callbacks are preserved across reset.
}

} // namespace atlas
