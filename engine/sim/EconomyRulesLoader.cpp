#include "sim/EconomyRulesLoader.h"
#include <sstream>
#include <algorithm>

namespace atlas::sim {

void EconomyRulesLoader::InstallDefaults() {
    // Global defaults
    {
        EconomyRules r;
        r.name                  = "Global Defaults";
        r.pirateSpawnRate       = 0.05f;
        r.minerSpawnRate        = 0.10f;
        r.haulerSpawnRate       = 0.08f;
        r.traderSpawnRate       = 0.06f;
        r.miningYieldMultiplier = 1.0f;
        r.resourceRegenRate     = 0.001f;
        r.threatDecayRate       = 0.002f;
        r.economicRecoveryRate  = 0.001f;
        r.trafficDriftRate      = 0.0005f;
        r.brokerFeePercent      = 3.0f;
        r.salesTaxPercent       = 2.0f;
        r.priceFloor            = 0.5f;
        r.priceCeiling          = 2.0f;
        r.highSecThreshold      = 0.5f;
        r.lowSecThreshold       = 0.1f;
        m_rules["global"] = r;
    }
    // High-sec rules — safer, more miners, fewer pirates
    {
        EconomyRules r;
        r.name                  = "High Security";
        r.pirateSpawnRate       = 0.01f;
        r.minerSpawnRate        = 0.15f;
        r.haulerSpawnRate       = 0.12f;
        r.traderSpawnRate       = 0.10f;
        r.miningYieldMultiplier = 0.8f;
        r.resourceRegenRate     = 0.002f;
        r.threatDecayRate       = 0.005f;
        r.economicRecoveryRate  = 0.002f;
        r.trafficDriftRate      = 0.001f;
        r.brokerFeePercent      = 3.0f;
        r.salesTaxPercent       = 2.0f;
        r.priceFloor            = 0.7f;
        r.priceCeiling          = 1.5f;
        r.highSecThreshold      = 0.5f;
        r.lowSecThreshold       = 0.1f;
        m_rules["highsec"] = r;
    }
    // Low-sec rules — riskier, better yields, more pirates
    {
        EconomyRules r;
        r.name                  = "Low Security";
        r.pirateSpawnRate       = 0.10f;
        r.minerSpawnRate        = 0.06f;
        r.haulerSpawnRate       = 0.04f;
        r.traderSpawnRate       = 0.03f;
        r.miningYieldMultiplier = 1.5f;
        r.resourceRegenRate     = 0.001f;
        r.threatDecayRate       = 0.001f;
        r.economicRecoveryRate  = 0.0005f;
        r.trafficDriftRate      = 0.0003f;
        r.brokerFeePercent      = 2.0f;
        r.salesTaxPercent       = 1.0f;
        r.priceFloor            = 0.3f;
        r.priceCeiling          = 3.0f;
        r.highSecThreshold      = 0.5f;
        r.lowSecThreshold       = 0.1f;
        m_rules["lowsec"] = r;
    }
    // Null-sec rules — lawless, highest yields, most pirates
    {
        EconomyRules r;
        r.name                  = "Null Security";
        r.pirateSpawnRate       = 0.15f;
        r.minerSpawnRate        = 0.03f;
        r.haulerSpawnRate       = 0.02f;
        r.traderSpawnRate       = 0.01f;
        r.miningYieldMultiplier = 2.0f;
        r.resourceRegenRate     = 0.0005f;
        r.threatDecayRate       = 0.0005f;
        r.economicRecoveryRate  = 0.0003f;
        r.trafficDriftRate      = 0.0001f;
        r.brokerFeePercent      = 1.0f;
        r.salesTaxPercent       = 0.0f;
        r.priceFloor            = 0.1f;
        r.priceCeiling          = 5.0f;
        r.highSecThreshold      = 0.5f;
        r.lowSecThreshold       = 0.1f;
        m_rules["nullsec"] = r;
    }
}

bool EconomyRulesLoader::AddRules(const std::string& id, const EconomyRules& rules) {
    m_rules[id] = rules;
    return true;
}

bool EconomyRulesLoader::RemoveRules(const std::string& id) {
    return m_rules.erase(id) > 0;
}

const EconomyRules* EconomyRulesLoader::GetRules(const std::string& id) const {
    auto it = m_rules.find(id);
    if (it == m_rules.end()) return nullptr;
    return &it->second;
}

std::vector<std::string> EconomyRulesLoader::RulesIds() const {
    std::vector<std::string> ids;
    ids.reserve(m_rules.size());
    for (auto& [key, val] : m_rules) {
        ids.push_back(key);
    }
    return ids;
}

const EconomyRules* EconomyRulesLoader::GetRulesForSecurity(float security) const {
    // Look up global thresholds to decide band
    const EconomyRules* global = GetRules("global");
    float highThresh = global ? global->highSecThreshold : 0.5f;
    float lowThresh  = global ? global->lowSecThreshold  : 0.1f;

    const char* band = nullptr;
    if (security >= highThresh) {
        band = "highsec";
    } else if (security >= lowThresh) {
        band = "lowsec";
    } else {
        band = "nullsec";
    }

    const EconomyRules* result = GetRules(band);
    if (result) return result;
    // Fallback to global
    return global;
}

// Trim leading/trailing whitespace
static std::string trimStr(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

size_t EconomyRulesLoader::LoadFromText(const std::string& text) {
    if (text.empty()) return 0;

    std::istringstream iss(text);
    std::string line;
    std::string currentId;
    EconomyRules currentRules;
    size_t count = 0;
    bool inBlock = false;

    auto commitBlock = [&]() {
        if (inBlock && !currentId.empty()) {
            m_rules[currentId] = currentRules;
            ++count;
        }
    };

    while (std::getline(iss, line)) {
        line = trimStr(line);
        if (line.empty()) continue;

        // Section header: [rule_id]
        if (line.front() == '[' && line.back() == ']') {
            commitBlock();
            currentId = trimStr(line.substr(1, line.size() - 2));
            currentRules = EconomyRules{};
            inBlock = true;
            continue;
        }

        if (!inBlock) continue;

        // Key = Value
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trimStr(line.substr(0, eq));
        std::string val = trimStr(line.substr(eq + 1));

        if      (key == "name")                  currentRules.name                  = val;
        else if (key == "pirateSpawnRate")        currentRules.pirateSpawnRate       = std::stof(val);
        else if (key == "minerSpawnRate")         currentRules.minerSpawnRate        = std::stof(val);
        else if (key == "haulerSpawnRate")        currentRules.haulerSpawnRate       = std::stof(val);
        else if (key == "traderSpawnRate")        currentRules.traderSpawnRate       = std::stof(val);
        else if (key == "miningYieldMultiplier")  currentRules.miningYieldMultiplier = std::stof(val);
        else if (key == "resourceRegenRate")      currentRules.resourceRegenRate     = std::stof(val);
        else if (key == "threatDecayRate")        currentRules.threatDecayRate       = std::stof(val);
        else if (key == "economicRecoveryRate")   currentRules.economicRecoveryRate  = std::stof(val);
        else if (key == "trafficDriftRate")       currentRules.trafficDriftRate      = std::stof(val);
        else if (key == "brokerFeePercent")       currentRules.brokerFeePercent      = std::stof(val);
        else if (key == "salesTaxPercent")        currentRules.salesTaxPercent       = std::stof(val);
        else if (key == "priceFloor")             currentRules.priceFloor            = std::stof(val);
        else if (key == "priceCeiling")           currentRules.priceCeiling          = std::stof(val);
        else if (key == "highSecThreshold")       currentRules.highSecThreshold      = std::stof(val);
        else if (key == "lowSecThreshold")        currentRules.lowSecThreshold       = std::stof(val);
    }

    commitBlock();
    return count;
}

} // namespace atlas::sim
