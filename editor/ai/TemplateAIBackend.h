#pragma once
#include "AIAggregator.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace atlas::ai {

/**
 * @brief Template-based AI backend for content generation suggestions.
 *
 * Uses a library of pre-defined prompt→response templates to provide
 * deterministic, offline AI assistance.  This backend is intended as a
 * fallback that works without any external API or network connection.
 *
 * Designers can register templates that match prompt keywords to
 * pre-authored responses.  When a prompt contains a registered keyword,
 * the matching response is returned with a base confidence score.
 *
 * Multiple keyword matches increase the confidence score, making the
 * response more likely to be selected by the AIAggregator.
 */
class TemplateAIBackend : public AIBackend {
public:
    TemplateAIBackend();
    ~TemplateAIBackend() override = default;

    AIResponse Query(const std::string& prompt,
                     const AIContext& context) override;

    // ── Template management ──────────────────────────────────────

    /** Register a keyword→response mapping with a base confidence. */
    void AddTemplate(const std::string& keyword,
                     const std::string& response,
                     float baseConfidence = 0.5f);

    /** Remove all templates for a keyword. */
    bool RemoveTemplate(const std::string& keyword);

    /** Number of registered templates. */
    size_t TemplateCount() const { return m_templates.size(); }

    /** Clear all templates. */
    void ClearTemplates();

    /** Install a set of built-in templates for common editor tasks. */
    void InstallDefaults();

private:
    struct Template {
        std::string keyword;
        std::string response;
        float baseConfidence;
    };

    std::vector<Template> m_templates;

    /** Convert a string to lowercase for case-insensitive matching. */
    static std::string toLower(const std::string& s);
};

} // namespace atlas::ai
