#include "sim/AIProfileLoader.h"
#include <sstream>
#include <algorithm>

namespace atlas::sim {

void AIProfileLoader::InstallDefaults() {
    // Miner
    {
        AIProfile p;
        p.name             = "Default Miner";
        p.archetype        = AIArchetype::Miner;
        p.orbitRange       = 2000.0f;
        p.retreatThreshold = 0.25f;
        p.focusFire        = false;
        p.cargoCapacity    = 400.0f;
        p.travelSpeed      = 100.0f;
        p.loadRate         = 2.0f;
        p.respawnDelay     = 60.0f;
        p.hitPoints        = 80.0f;
        p.startingCredits  = 2000.0f;
        m_profiles["miner_default"] = p;
    }
    // Hauler
    {
        AIProfile p;
        p.name             = "Default Hauler";
        p.archetype        = AIArchetype::Hauler;
        p.orbitRange       = 3000.0f;
        p.retreatThreshold = 0.30f;
        p.focusFire        = false;
        p.cargoCapacity    = 1000.0f;
        p.travelSpeed      = 80.0f;
        p.loadRate         = 5.0f;
        p.respawnDelay     = 90.0f;
        p.hitPoints        = 120.0f;
        p.startingCredits  = 8000.0f;
        m_profiles["hauler_default"] = p;
    }
    // Trader
    {
        AIProfile p;
        p.name             = "Default Trader";
        p.archetype        = AIArchetype::Trader;
        p.orbitRange       = 4000.0f;
        p.retreatThreshold = 0.35f;
        p.focusFire        = false;
        p.cargoCapacity    = 500.0f;
        p.travelSpeed      = 90.0f;
        p.loadRate         = 3.0f;
        p.respawnDelay     = 90.0f;
        p.hitPoints        = 80.0f;
        p.startingCredits  = 5000.0f;
        m_profiles["trader_default"] = p;
    }
    // Pirate
    {
        AIProfile p;
        p.name             = "Default Pirate";
        p.archetype        = AIArchetype::Pirate;
        p.orbitRange       = 5000.0f;
        p.retreatThreshold = 0.20f;
        p.focusFire        = true;
        p.cargoCapacity    = 200.0f;
        p.travelSpeed      = 120.0f;
        p.loadRate         = 1.0f;
        p.respawnDelay     = 120.0f;
        p.hitPoints        = 100.0f;
        p.startingCredits  = 1000.0f;
        m_profiles["pirate_default"] = p;
    }
    // Security
    {
        AIProfile p;
        p.name             = "Default Security";
        p.archetype        = AIArchetype::Security;
        p.orbitRange       = 6000.0f;
        p.retreatThreshold = 0.15f;
        p.focusFire        = true;
        p.cargoCapacity    = 50.0f;
        p.travelSpeed      = 110.0f;
        p.loadRate         = 0.0f;
        p.respawnDelay     = 45.0f;
        p.hitPoints        = 150.0f;
        p.startingCredits  = 0.0f;
        m_profiles["security_default"] = p;
    }
    // Industrialist
    {
        AIProfile p;
        p.name             = "Default Industrialist";
        p.archetype        = AIArchetype::Industrialist;
        p.orbitRange       = 2000.0f;
        p.retreatThreshold = 0.40f;
        p.focusFire        = false;
        p.cargoCapacity    = 300.0f;
        p.travelSpeed      = 70.0f;
        p.loadRate         = 4.0f;
        p.respawnDelay     = 120.0f;
        p.hitPoints        = 60.0f;
        p.startingCredits  = 10000.0f;
        m_profiles["industrialist_default"] = p;
    }
}

bool AIProfileLoader::AddProfile(const std::string& id, const AIProfile& profile) {
    m_profiles[id] = profile;
    return true;
}

bool AIProfileLoader::RemoveProfile(const std::string& id) {
    return m_profiles.erase(id) > 0;
}

const AIProfile* AIProfileLoader::GetProfile(const std::string& id) const {
    auto it = m_profiles.find(id);
    if (it == m_profiles.end()) return nullptr;
    return &it->second;
}

std::vector<const AIProfile*> AIProfileLoader::GetByArchetype(AIArchetype archetype) const {
    std::vector<const AIProfile*> result;
    for (auto& [key, prof] : m_profiles) {
        if (prof.archetype == archetype) {
            result.push_back(&prof);
        }
    }
    return result;
}

std::vector<std::string> AIProfileLoader::ProfileIds() const {
    std::vector<std::string> ids;
    ids.reserve(m_profiles.size());
    for (auto& [key, val] : m_profiles) {
        ids.push_back(key);
    }
    return ids;
}

// Trim leading/trailing whitespace
static std::string trimStr(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static AIArchetype parseArchetype(const std::string& s) {
    if (s == "Miner")          return AIArchetype::Miner;
    if (s == "Hauler")         return AIArchetype::Hauler;
    if (s == "Trader")         return AIArchetype::Trader;
    if (s == "Pirate")         return AIArchetype::Pirate;
    if (s == "Security")       return AIArchetype::Security;
    if (s == "Industrialist")  return AIArchetype::Industrialist;
    return AIArchetype::Miner; // fallback
}

size_t AIProfileLoader::LoadFromText(const std::string& text) {
    if (text.empty()) return 0;

    std::istringstream iss(text);
    std::string line;
    std::string currentId;
    AIProfile   currentProfile;
    size_t      count = 0;
    bool        inBlock = false;

    auto commitBlock = [&]() {
        if (inBlock && !currentId.empty()) {
            m_profiles[currentId] = currentProfile;
            ++count;
        }
    };

    while (std::getline(iss, line)) {
        line = trimStr(line);
        if (line.empty()) continue;

        // Section header: [profile_id]
        if (line.front() == '[' && line.back() == ']') {
            commitBlock();
            currentId = trimStr(line.substr(1, line.size() - 2));
            currentProfile = AIProfile{};
            inBlock = true;
            continue;
        }

        if (!inBlock) continue;

        // Key = Value
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trimStr(line.substr(0, eq));
        std::string val = trimStr(line.substr(eq + 1));

        if      (key == "name")             currentProfile.name             = val;
        else if (key == "archetype")        currentProfile.archetype        = parseArchetype(val);
        else if (key == "orbitRange")        currentProfile.orbitRange       = std::stof(val);
        else if (key == "retreatThreshold")  currentProfile.retreatThreshold = std::stof(val);
        else if (key == "focusFire")         currentProfile.focusFire        = (val == "true" || val == "1");
        else if (key == "cargoCapacity")     currentProfile.cargoCapacity    = std::stof(val);
        else if (key == "travelSpeed")       currentProfile.travelSpeed      = std::stof(val);
        else if (key == "loadRate")           currentProfile.loadRate         = std::stof(val);
        else if (key == "respawnDelay")      currentProfile.respawnDelay     = std::stof(val);
        else if (key == "hitPoints")         currentProfile.hitPoints        = std::stof(val);
        else if (key == "startingCredits")   currentProfile.startingCredits  = std::stof(val);
    }

    commitBlock();
    return count;
}

} // namespace atlas::sim
