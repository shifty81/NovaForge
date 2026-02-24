#include "AIAggregator.h"

namespace atlas::ai {

void AIAggregator::RegisterBackend(AIBackend* backend) {
    if (backend) {
        m_backends.push_back(backend);
    }
}

AIResponse AIAggregator::Execute(
    AIRequestType /*type*/,
    const std::string& prompt,
    const AIContext& context) {
    std::vector<AIResponse> responses;
    for (auto* backend : m_backends) {
        responses.push_back(backend->Query(prompt, context));
    }
    return SelectBest(responses);
}

AIResponse AIAggregator::SelectBest(const std::vector<AIResponse>& responses) const {
    if (responses.empty()) {
        return AIResponse{};
    }
    const AIResponse* best = &responses[0];
    for (size_t i = 1; i < responses.size(); ++i) {
        if (responses[i].confidence > best->confidence) {
            best = &responses[i];
        }
    }
    return *best;
}

}
