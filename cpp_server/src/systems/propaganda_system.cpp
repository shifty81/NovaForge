#include "systems/propaganda_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <sstream>
#include <random>
#include <memory>

namespace atlas {
namespace systems {

PropagandaSystem::PropagandaSystem(ecs::World* world)
    : System(world) {
}

void PropagandaSystem::update(float delta_time) {
    // Decay credibility of all myths over time
    auto entities = world_->getEntities<components::PropagandaNetwork>();
    for (auto* entity : entities) {
        auto* network = entity->getComponent<components::PropagandaNetwork>();
        if (!network) continue;

        for (auto& myth : network->myths) {
            if (!myth.debunked && myth.credibility > 0.0f) {
                myth.credibility = std::max(0.0f, myth.credibility - credibility_decay_rate_ * delta_time);
                if (myth.credibility <= 0.0f) {
                    myth.debunked = true;
                }
            }
        }
    }
}

std::string PropagandaSystem::generateMythContent(const std::string& subject_id,
                                                   const std::string& myth_type,
                                                   const std::string& base_event) {
    std::ostringstream oss;

    if (myth_type == "heroic") {
        oss << "Tales speak of " << subject_id << " performing legendary feats of valor";
        if (!base_event.empty()) {
            oss << " during " << base_event;
        }
        oss << ". Some say they single-handedly saved an entire station.";
    } else if (myth_type == "villainous") {
        oss << "Whispers tell of " << subject_id << "'s dark deeds";
        if (!base_event.empty()) {
            oss << " connected to " << base_event;
        }
        oss << ". They say no ship is safe in their presence.";
    } else if (myth_type == "mysterious") {
        oss << "Strange rumors circulate about " << subject_id;
        if (!base_event.empty()) {
            oss << " and their involvement in " << base_event;
        }
        oss << ". What secrets do they hide?";
    } else if (myth_type == "exaggerated") {
        oss << "The story of " << subject_id;
        if (!base_event.empty()) {
            oss << " and " << base_event;
        }
        oss << " has grown with each telling. Ten ships? No, a hundred!";
    } else {
        oss << "A tale is told of " << subject_id;
        if (!base_event.empty()) {
            oss << " regarding " << base_event;
        }
        oss << ", though none can verify its truth.";
    }

    return oss.str();
}

std::string PropagandaSystem::generateMyth(const std::string& subject_id,
                                            const std::string& source_faction,
                                            const std::string& myth_type,
                                            const std::string& base_event) {
    // Get or create global propaganda network
    auto entities = world_->getEntities<components::PropagandaNetwork>();
    components::PropagandaNetwork* network = nullptr;

    for (auto* entity : entities) {
        network = entity->getComponent<components::PropagandaNetwork>();
        if (network) break;
    }

    if (!network) {
        auto* entity = world_->createEntity("propaganda_network");
        auto net = std::make_unique<components::PropagandaNetwork>();
        network = net.get();
        entity->addComponent(std::move(net));
    }

    // Create the myth
    components::PropagandaNetwork::MythEntry myth;
    myth.myth_id = "myth_" + std::to_string(++myth_counter_);
    myth.subject_id = subject_id;
    myth.source_faction = source_faction;
    myth.base_event = base_event;
    myth.credibility = 1.0f;
    myth.spread_count = 1;
    myth.debunked = false;

    // Set type
    if (myth_type == "heroic") {
        myth.type = components::PropagandaNetwork::MythType::Heroic;
    } else if (myth_type == "villainous") {
        myth.type = components::PropagandaNetwork::MythType::Villainous;
    } else if (myth_type == "mysterious") {
        myth.type = components::PropagandaNetwork::MythType::Mysterious;
    } else if (myth_type == "exaggerated") {
        myth.type = components::PropagandaNetwork::MythType::Exaggerated;
    } else {
        myth.type = components::PropagandaNetwork::MythType::Fabricated;
    }

    myth.content = generateMythContent(subject_id, myth_type, base_event);

    // Trim old myths if at capacity
    while (static_cast<int>(network->myths.size()) >= network->max_myths) {
        network->myths.erase(network->myths.begin());
    }

    network->myths.push_back(myth);
    return myth.myth_id;
}

void PropagandaSystem::spreadMyth(const std::string& myth_id,
                                   const std::string& /*target_system_id*/,
                                   float spread_factor) {
    auto entities = world_->getEntities<components::PropagandaNetwork>();
    for (auto* entity : entities) {
        auto* network = entity->getComponent<components::PropagandaNetwork>();
        if (!network) continue;

        auto* myth = network->findMyth(myth_id);
        if (myth && !myth->debunked) {
            myth->spread_count++;
            // Spreading can slightly reduce credibility (story gets distorted)
            myth->credibility = std::max(0.1f, myth->credibility - spread_factor * 0.05f);
        }
    }
}

std::vector<components::PropagandaNetwork::MythEntry>
PropagandaSystem::getMythsAbout(const std::string& subject_id,
                                 bool include_debunked) const {
    std::vector<components::PropagandaNetwork::MythEntry> result;
    auto entities = world_->getEntities<components::PropagandaNetwork>();

    for (auto* entity : entities) {
        auto* network = entity->getComponent<components::PropagandaNetwork>();
        if (!network) continue;

        auto myths = network->getMythsAbout(subject_id, include_debunked);
        result.insert(result.end(), myths.begin(), myths.end());
    }

    return result;
}

float PropagandaSystem::debunkMyth(const std::string& myth_id, float evidence_strength) {
    auto entities = world_->getEntities<components::PropagandaNetwork>();
    for (auto* entity : entities) {
        auto* network = entity->getComponent<components::PropagandaNetwork>();
        if (!network) continue;

        auto* myth = network->findMyth(myth_id);
        if (myth) {
            myth->credibility = std::max(0.0f, myth->credibility - evidence_strength);
            if (myth->credibility <= 0.0f) {
                myth->debunked = true;
            }
            return myth->credibility;
        }
    }
    return 0.0f;
}

float PropagandaSystem::getMythCredibility(const std::string& myth_id) const {
    auto entities = world_->getEntities<components::PropagandaNetwork>();
    for (auto* entity : entities) {
        auto* network = entity->getComponent<components::PropagandaNetwork>();
        if (!network) continue;

        auto* myth = network->findMyth(myth_id);
        if (myth) {
            return myth->credibility;
        }
    }
    return 0.0f;
}

bool PropagandaSystem::npcBelievesMyth(const std::string& /*npc_id*/,
                                        const std::string& myth_id) const {
    // NPCs believe myths with credibility > 0.3
    float cred = getMythCredibility(myth_id);
    return cred > 0.3f;
}

int PropagandaSystem::getActiveMythCount(const std::string& subject_id) const {
    auto myths = getMythsAbout(subject_id, false);
    int count = 0;
    for (const auto& m : myths) {
        if (!m.debunked && m.credibility > 0.0f) count++;
    }
    return count;
}

std::string PropagandaSystem::getMythTypeName(int type_index) {
    switch (type_index) {
        case 0: return "Heroic";
        case 1: return "Villainous";
        case 2: return "Mysterious";
        case 3: return "Exaggerated";
        case 4: return "Fabricated";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
