#include "ModelImportPanel.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

ModelImportPanel::ModelImportPanel() = default;

// ── Scanning ─────────────────────────────────────────────────────────

int ModelImportPanel::ScanDirectory(const std::string& directory) {
    int added = 0;
    namespace fs = std::filesystem;

    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        log("Scan: directory not found — " + directory);
        return 0;
    }

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        // Lowercase the extension
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        std::string format;
        if (ext == ".obj")       format = "obj";
        else if (ext == ".gltf") format = "gltf";
        else if (ext == ".glb")  format = "glb";
        else continue;

        std::string stem = entry.path().stem().string();

        // Skip if already in catalogue
        if (findModel(stem)) continue;

        ModelCatalogEntry model;
        model.modelId  = stem;
        model.filename = entry.path().filename().string();
        model.format   = format;
        model.fileSize = static_cast<int64_t>(entry.file_size());

        // Auto-categorise by name heuristic
        std::string lower = stem;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower.find("turret") != std::string::npos) model.category = "turret";
        else if (lower.find("engine") != std::string::npos) model.category = "engine";
        else if (lower.find("core") != std::string::npos) model.category = "core";
        else if (lower.find("wing") != std::string::npos) model.category = "wing";
        else if (lower.find("weapon") != std::string::npos) model.category = "weapon";
        else if (lower.find("spine") != std::string::npos) model.category = "hull";
        else if (lower.find("hull") != std::string::npos) model.category = "hull";
        else if (lower.find("hangar") != std::string::npos) model.category = "prop";
        else model.category = "prop";

        // Quick OBJ vertex/face count from header scan
        if (format == "obj") {
            std::ifstream f(entry.path());
            if (f.is_open()) {
                std::string line;
                int verts = 0, faces = 0;
                while (std::getline(f, line)) {
                    if (line.size() > 2) {
                        if (line[0] == 'v' && line[1] == ' ') ++verts;
                        else if (line[0] == 'f' && line[1] == ' ') ++faces;
                    }
                }
                model.vertexCount = verts;
                model.faceCount   = faces;
                model.validated   = true;
            }
        }

        m_models.push_back(model);
        ++added;
    }

    log("Scanned " + directory + " — found " + std::to_string(added) + " models");
    return added;
}

// ── Catalogue management ─────────────────────────────────────────────

bool ModelImportPanel::AddModel(const ModelCatalogEntry& entry) {
    if (entry.modelId.empty()) return false;
    if (findModel(entry.modelId)) return false;  // duplicate
    m_models.push_back(entry);
    log("Added model: " + entry.modelId + " (" + entry.format + ")");
    return true;
}

bool ModelImportPanel::RemoveModel(const std::string& modelId) {
    auto it = std::find_if(m_models.begin(), m_models.end(),
        [&modelId](const ModelCatalogEntry& e) { return e.modelId == modelId; });
    if (it == m_models.end()) return false;

    log("Removed model: " + modelId);
    m_models.erase(it);

    // Fix selection
    if (m_selectedIndex >= static_cast<int>(m_models.size())) {
        m_selectedIndex = -1;
    }
    return true;
}

const ModelCatalogEntry* ModelImportPanel::GetModel(const std::string& modelId) const {
    return findModel(modelId);
}

// ── Category / tags ──────────────────────────────────────────────────

bool ModelImportPanel::SetCategory(const std::string& modelId, const std::string& category) {
    auto* model = findModel(modelId);
    if (!model) return false;
    model->category = category;
    log("Set category: " + modelId + " → " + category);
    return true;
}

bool ModelImportPanel::SetTags(const std::string& modelId, const std::string& tags) {
    auto* model = findModel(modelId);
    if (!model) return false;
    model->tags = tags;
    return true;
}

// ── Filtering ────────────────────────────────────────────────────────

void ModelImportPanel::SetCategoryFilter(const std::string& category) {
    m_categoryFilter = category;
}

void ModelImportPanel::SetSearchFilter(const std::string& search) {
    m_searchFilter = search;
}

int ModelImportPanel::FilteredCount() const {
    int count = 0;
    for (const auto& model : m_models) {
        if (matchesFilters(model)) ++count;
    }
    return count;
}

// ── Selection ────────────────────────────────────────────────────────

void ModelImportPanel::SelectModel(int index) {
    if (index >= 0 && index < static_cast<int>(m_models.size())) {
        m_selectedIndex = index;
    }
}

void ModelImportPanel::ClearSelection() {
    m_selectedIndex = -1;
}

// ── Validation ───────────────────────────────────────────────────────

bool ModelImportPanel::ValidateModel(const std::string& modelId) {
    auto* model = findModel(modelId);
    if (!model) return false;
    model->validated = !model->filename.empty() && model->fileSize > 0;
    return model->validated;
}

int ModelImportPanel::ValidateAll() {
    int valid = 0;
    for (auto& model : m_models) {
        model.validated = !model.filename.empty() && model.fileSize > 0;
        if (model.validated) ++valid;
    }
    log("Validated " + std::to_string(valid) + "/" + std::to_string(m_models.size()) + " models");
    return valid;
}

// ── Export / Import ──────────────────────────────────────────────────

std::string ModelImportPanel::ExportToJson() const {
    std::ostringstream os;
    os << "{\n  \"models\": [\n";
    for (size_t i = 0; i < m_models.size(); ++i) {
        const auto& m = m_models[i];
        os << "    {\n";
        os << "      \"modelId\": \"" << m.modelId << "\",\n";
        os << "      \"filename\": \"" << m.filename << "\",\n";
        os << "      \"category\": \"" << m.category << "\",\n";
        os << "      \"format\": \"" << m.format << "\",\n";
        os << "      \"tags\": \"" << m.tags << "\",\n";
        os << "      \"vertexCount\": " << m.vertexCount << ",\n";
        os << "      \"faceCount\": " << m.faceCount << ",\n";
        os << "      \"fileSize\": " << m.fileSize << ",\n";
        os << "      \"validated\": " << (m.validated ? "true" : "false") << "\n";
        os << "    }";
        if (i + 1 < m_models.size()) os << ",";
        os << "\n";
    }
    os << "  ]\n}";
    return os.str();
}

int ModelImportPanel::ImportFromJson(const std::string& json) {
    // Minimal parser: extract model entries from JSON array
    int imported = 0;
    size_t pos = 0;

    while ((pos = json.find("\"modelId\"", pos)) != std::string::npos) {
        ModelCatalogEntry entry;

        // Parse modelId
        auto extractStr = [&](const std::string& key, size_t from) -> std::string {
            size_t kpos = json.find("\"" + key + "\"", from);
            if (kpos == std::string::npos) return "";
            size_t colon = json.find(':', kpos);
            if (colon == std::string::npos) return "";
            size_t q1 = json.find('"', colon + 1);
            if (q1 == std::string::npos) return "";
            size_t q2 = json.find('"', q1 + 1);
            if (q2 == std::string::npos) return "";
            return json.substr(q1 + 1, q2 - q1 - 1);
        };

        auto extractInt = [&](const std::string& key, size_t from) -> int {
            size_t kpos = json.find("\"" + key + "\"", from);
            if (kpos == std::string::npos) return 0;
            size_t colon = json.find(':', kpos);
            if (colon == std::string::npos) return 0;
            size_t start = colon + 1;
            while (start < json.size() && (json[start] == ' ' || json[start] == '\t')) ++start;
            return std::atoi(json.c_str() + start);
        };

        entry.modelId   = extractStr("modelId", pos);
        entry.filename  = extractStr("filename", pos);
        entry.category  = extractStr("category", pos);
        entry.format    = extractStr("format", pos);
        entry.tags      = extractStr("tags", pos);
        entry.vertexCount = extractInt("vertexCount", pos);
        entry.faceCount   = extractInt("faceCount", pos);
        entry.fileSize    = extractInt("fileSize", pos);

        std::string validStr = extractStr("validated", pos);
        // validated is a boolean, handle as non-string too
        size_t vpos = json.find("\"validated\"", pos);
        if (vpos != std::string::npos) {
            size_t colon = json.find(':', vpos);
            if (colon != std::string::npos) {
                std::string rest = json.substr(colon + 1, 10);
                entry.validated = rest.find("true") != std::string::npos;
            }
        }

        if (!entry.modelId.empty() && !findModel(entry.modelId)) {
            m_models.push_back(entry);
            ++imported;
        }

        pos += 10;  // advance past this entry
    }

    if (imported > 0) {
        log("Imported " + std::to_string(imported) + " models from JSON");
    }
    return imported;
}

// ── Statistics ────────────────────────────────────────────────────────

int ModelImportPanel::TotalVertices() const {
    int total = 0;
    for (const auto& m : m_models) total += m.vertexCount;
    return total;
}

int ModelImportPanel::TotalFaces() const {
    int total = 0;
    for (const auto& m : m_models) total += m.faceCount;
    return total;
}

int ModelImportPanel::ValidatedCount() const {
    int count = 0;
    for (const auto& m : m_models) {
        if (m.validated) ++count;
    }
    return count;
}

// ── Draw ──────────────────────────────────────────────────────────────

void ModelImportPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Model Import", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Summary line
    std::string summary = "Models: " + std::to_string(ModelCount())
        + "  Vertices: " + std::to_string(TotalVertices())
        + "  Faces: " + std::to_string(TotalFaces())
        + "  Validated: " + std::to_string(ValidatedCount());
    atlas::label(ctx, {b.x + pad, y}, summary, ctx.theme().textPrimary);
    y += rowH + pad;

    // Model listing
    static constexpr int MAX_DISPLAYED = 12;
    int displayed = 0;
    for (int i = 0; i < static_cast<int>(m_models.size()) && displayed < MAX_DISPLAYED; ++i) {
        if (!matchesFilters(m_models[i])) continue;
        const auto& m = m_models[i];
        std::string info = m.modelId + "  [" + m.category + "]  "
            + m.format + "  V:" + std::to_string(m.vertexCount)
            + " F:" + std::to_string(m.faceCount);
        const atlas::Color& color = (i == m_selectedIndex)
            ? ctx.theme().accentPrimary : ctx.theme().textPrimary;
        atlas::label(ctx, {b.x + pad, y}, info, color);
        y += rowH;
        ++displayed;
    }
    y += pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Helpers ───────────────────────────────────────────────────────────

void ModelImportPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

bool ModelImportPanel::matchesFilters(const ModelCatalogEntry& entry) const {
    if (!m_categoryFilter.empty() && entry.category != m_categoryFilter) return false;
    if (!m_searchFilter.empty()) {
        std::string lower = entry.modelId + " " + entry.tags;
        std::string filter = m_searchFilter;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::transform(filter.begin(), filter.end(), filter.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower.find(filter) == std::string::npos) return false;
    }
    return true;
}

ModelCatalogEntry* ModelImportPanel::findModel(const std::string& modelId) {
    for (auto& m : m_models) {
        if (m.modelId == modelId) return &m;
    }
    return nullptr;
}

const ModelCatalogEntry* ModelImportPanel::findModel(const std::string& modelId) const {
    for (const auto& m : m_models) {
        if (m.modelId == modelId) return &m;
    }
    return nullptr;
}

} // namespace atlas::editor
