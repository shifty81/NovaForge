#include "TemplateAIBackend.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace atlas::ai {

// ── Construction ───────────────────────────────────────────────────

TemplateAIBackend::TemplateAIBackend() {
    InstallDefaults();
}

// ── Query ──────────────────────────────────────────────────────────

AIResponse TemplateAIBackend::Query(const std::string& prompt,
                                     const AIContext& /*context*/) {
    std::string lowerPrompt = toLower(prompt);

    // Collect all matching templates.
    std::vector<const Template*> matches;
    for (const auto& t : m_templates) {
        if (lowerPrompt.find(toLower(t.keyword)) != std::string::npos) {
            matches.push_back(&t);
        }
    }

    if (matches.empty()) {
        return AIResponse{"No matching template found", 0.0f};
    }

    // Pick the best match (highest base confidence).
    const Template* best = matches[0];
    for (size_t i = 1; i < matches.size(); ++i) {
        if (matches[i]->baseConfidence > best->baseConfidence) {
            best = matches[i];
        }
    }

    // Boost confidence slightly based on number of keyword hits.
    float conf = best->baseConfidence;
    float boost = static_cast<float>(matches.size() - 1) * 0.05f;
    conf = std::min(conf + boost, 1.0f);

    return AIResponse{best->response, conf};
}

// ── Template management ────────────────────────────────────────────

void TemplateAIBackend::AddTemplate(const std::string& keyword,
                                     const std::string& response,
                                     float baseConfidence) {
    if (keyword.empty()) return;
    m_templates.push_back({keyword, response, baseConfidence});
}

bool TemplateAIBackend::RemoveTemplate(const std::string& keyword) {
    std::string lk = toLower(keyword);
    auto it = std::remove_if(m_templates.begin(), m_templates.end(),
        [&lk](const Template& t) { return toLower(t.keyword) == lk; });
    if (it == m_templates.end()) return false;
    m_templates.erase(it, m_templates.end());
    return true;
}

void TemplateAIBackend::ClearTemplates() {
    m_templates.clear();
}

// ── Default templates ──────────────────────────────────────────────

void TemplateAIBackend::InstallDefaults() {
    // Ship generation suggestions
    AddTemplate("frigate",
        "Recommended frigate configuration: light armor, "
        "2 high slots (small turrets), 3 mid slots (shield/prop), "
        "2 low slots (damage mods). Suggest 30m signature radius.",
        0.6f);

    AddTemplate("cruiser",
        "Recommended cruiser configuration: medium armor plate, "
        "4 high slots (medium turrets/launchers), 4 mid slots, "
        "4 low slots. Suggest 120m signature radius.",
        0.6f);

    AddTemplate("battleship",
        "Recommended battleship configuration: heavy armor/shield tank, "
        "6-8 high slots (large turrets), 5 mid slots, 6 low slots. "
        "Suggest 400m signature radius.",
        0.6f);

    // Mission suggestions
    AddTemplate("combat mission",
        "Suggested combat mission: 3-5 waves of hostile NPCs, "
        "escalating difficulty. Objectives: destroy all hostiles. "
        "Reward: 150K-500K ISC based on level. Time limit: 1 hour.",
        0.7f);

    AddTemplate("mining mission",
        "Suggested mining mission: mine 500-2000 units of ore "
        "from designated asteroid belt. Reward: 100K-300K ISC. "
        "Time limit: 2 hours. Bonus for rare ore types.",
        0.7f);

    AddTemplate("courier mission",
        "Suggested courier mission: transport 100-1000 m3 of cargo "
        "between two stations (1-3 jumps). Reward: 50K-200K ISC. "
        "Time limit: 30 minutes per jump.",
        0.7f);

    AddTemplate("exploration",
        "Suggested exploration mission: scan down 1-3 cosmic signatures "
        "in target system. Hack relic/data sites for loot. "
        "Reward: 200K-800K ISC. No time limit.",
        0.65f);

    // Formation suggestions
    AddTemplate("formation",
        "Recommended fleet formation: Arrow for aggressive pushes, "
        "Line for broadside engagements, Wedge for defensive posture, "
        "Spread for area control. 500m base spacing for subcapitals.",
        0.55f);

    // PCG suggestions
    AddTemplate("station",
        "Suggested station generation: 4-8 modules for small station, "
        "12-20 for medium, 30+ for large. Include docking ring, "
        "market hub, repair bay, and defense turrets.",
        0.6f);

    AddTemplate("pirate",
        "Pirate faction profile: angular/aggressive hull lines, "
        "dark red/black color scheme, mismatched salvaged parts, "
        "bonus to speed and damage, penalty to tank.",
        0.6f);
}

// ── Utility ────────────────────────────────────────────────────────

std::string TemplateAIBackend::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

} // namespace atlas::ai
