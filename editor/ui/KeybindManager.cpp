#include "KeybindManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

KeybindManager::KeybindManager() {
    InstallDefaults();
}

// ── Binding management ─────────────────────────────────────────────

bool KeybindManager::AddBinding(const Keybind& binding) {
    if (binding.action.empty()) return false;
    // Reject duplicate action names.
    if (findMutable(binding.action)) return false;
    m_bindings.push_back(binding);
    return true;
}

bool KeybindManager::RemoveBinding(const std::string& action) {
    auto it = std::remove_if(m_bindings.begin(), m_bindings.end(),
        [&action](const Keybind& kb) { return kb.action == action; });
    if (it == m_bindings.end()) return false;
    m_bindings.erase(it, m_bindings.end());
    // Also remove callback.
    m_callbacks.erase(
        std::remove_if(m_callbacks.begin(), m_callbacks.end(),
            [&action](const CallbackEntry& ce) { return ce.action == action; }),
        m_callbacks.end());
    return true;
}

bool KeybindManager::Rebind(const std::string& action, int key, KeyMod mods) {
    Keybind* kb = findMutable(action);
    if (!kb) return false;
    kb->key  = key;
    kb->mods = mods;
    return true;
}

bool KeybindManager::SetEnabled(const std::string& action, bool enabled) {
    Keybind* kb = findMutable(action);
    if (!kb) return false;
    kb->enabled = enabled;
    return true;
}

const Keybind* KeybindManager::FindBinding(const std::string& action) const {
    for (const auto& kb : m_bindings) {
        if (kb.action == action) return &kb;
    }
    return nullptr;
}

std::vector<const Keybind*> KeybindManager::FindByKey(int key, KeyMod mods) const {
    std::vector<const Keybind*> result;
    for (const auto& kb : m_bindings) {
        if (kb.key == key && kb.mods == mods) {
            result.push_back(&kb);
        }
    }
    return result;
}

std::vector<const Keybind*> KeybindManager::BindingsInCategory(
    const std::string& category) const {
    std::vector<const Keybind*> result;
    for (const auto& kb : m_bindings) {
        if (kb.category == category) {
            result.push_back(&kb);
        }
    }
    return result;
}

// ── Conflict detection ─────────────────────────────────────────────

bool KeybindManager::HasConflict(int key, KeyMod mods,
                                  const std::string& excludeAction) const {
    for (const auto& kb : m_bindings) {
        if (kb.key == key && kb.mods == mods && kb.action != excludeAction) {
            return true;
        }
    }
    return false;
}

// ── Action dispatch ────────────────────────────────────────────────

void KeybindManager::RegisterCallback(const std::string& action,
                                       ActionCallback callback) {
    // Replace existing callback or add new one.
    for (auto& ce : m_callbacks) {
        if (ce.action == action) {
            ce.callback = std::move(callback);
            return;
        }
    }
    m_callbacks.push_back({action, std::move(callback)});
}

bool KeybindManager::HandleKeyPress(int key, KeyMod mods) {
    for (const auto& kb : m_bindings) {
        if (kb.key == key && kb.mods == mods && kb.enabled) {
            for (const auto& ce : m_callbacks) {
                if (ce.action == kb.action && ce.callback) {
                    ce.callback();
                    return true;
                }
            }
        }
    }
    return false;
}

// ── Display helpers ────────────────────────────────────────────────

std::string KeybindManager::DescribeBinding(int key, KeyMod mods) {
    std::string desc;
    if (hasFlag(mods, KeyMod::Ctrl))  desc += "Ctrl+";
    if (hasFlag(mods, KeyMod::Shift)) desc += "Shift+";
    if (hasFlag(mods, KeyMod::Alt))   desc += "Alt+";

    // Common named keys.
    if (key == 127 || key == 8) desc += "Delete";
    else if (key == 27)         desc += "Escape";
    else if (key == 9)          desc += "Tab";
    else if (key == 13)         desc += "Enter";
    else if (key == 32)         desc += "Space";
    else if (key >= 'A' && key <= 'Z') desc += static_cast<char>(key);
    else if (key >= 'a' && key <= 'z') desc += static_cast<char>(key - 32); // uppercase
    else if (key >= '0' && key <= '9') desc += static_cast<char>(key);
    else desc += "Key(" + std::to_string(key) + ")";

    return desc;
}

std::string KeybindManager::DescribeAction(const std::string& action) const {
    const Keybind* kb = FindBinding(action);
    if (!kb) return action + " (unbound)";
    return action + " [" + DescribeBinding(kb->key, kb->mods) + "]";
}

// ── Default bindings ───────────────────────────────────────────────

void KeybindManager::InstallDefaults() {
    // General
    AddBinding({"Undo",       "General", 'Z', KeyMod::Ctrl});
    AddBinding({"Redo",       "General", 'Y', KeyMod::Ctrl});
    AddBinding({"Save",       "General", 'S', KeyMod::Ctrl});
    AddBinding({"Delete",     "General", 127, KeyMod::None});  // Delete key

    // Viewport
    AddBinding({"Translate",  "Viewport", 'W', KeyMod::None});
    AddBinding({"Rotate",     "Viewport", 'E', KeyMod::None});
    AddBinding({"Scale",      "Viewport", 'R', KeyMod::None});
    AddBinding({"ToggleGrid", "Viewport", 'G', KeyMod::None});
    AddBinding({"FocusSelected", "Viewport", 'F', KeyMod::None});

    // Panels
    AddBinding({"ToggleConsole",    "Panels", '`', KeyMod::None});
    AddBinding({"ToggleInspector",  "Panels", 'I', KeyMod::Ctrl});
    AddBinding({"ToggleSceneGraph", "Panels", 'H', KeyMod::Ctrl});
}

void KeybindManager::Clear() {
    m_bindings.clear();
    m_callbacks.clear();
}

// ── Internals ──────────────────────────────────────────────────────

Keybind* KeybindManager::findMutable(const std::string& action) {
    for (auto& kb : m_bindings) {
        if (kb.action == action) return &kb;
    }
    return nullptr;
}

// ── Serialisation ──────────────────────────────────────────────────

std::string KeybindManager::SerializeToJSON() const {
    std::ostringstream os;
    os << "{\n  \"keybinds\": [\n";
    for (size_t i = 0; i < m_bindings.size(); ++i) {
        const auto& kb = m_bindings[i];
        os << "    {\n"
           << "      \"action\": \""   << kb.action   << "\",\n"
           << "      \"category\": \"" << kb.category  << "\",\n"
           << "      \"key\": "        << kb.key       << ",\n"
           << "      \"mods\": "       << static_cast<int>(kb.mods) << ",\n"
           << "      \"enabled\": "    << (kb.enabled ? "true" : "false") << "\n"
           << "    }";
        if (i + 1 < m_bindings.size()) os << ",";
        os << "\n";
    }
    os << "  ]\n}\n";
    return os.str();
}

bool KeybindManager::DeserializeFromJSON(const std::string& json) {
    std::vector<Keybind> loaded;

    size_t pos = 0;
    while ((pos = json.find("\"action\"", pos)) != std::string::npos) {
        Keybind kb;

        // action
        size_t colon = json.find(':', pos);
        if (colon == std::string::npos) break;
        size_t q1 = json.find('\"', colon + 1);
        size_t q2 = (q1 != std::string::npos) ? json.find('\"', q1 + 1) : std::string::npos;
        if (q1 == std::string::npos || q2 == std::string::npos) break;
        kb.action = json.substr(q1 + 1, q2 - q1 - 1);

        // category
        size_t catKey = json.find("\"category\"", pos);
        if (catKey != std::string::npos) {
            size_t cc = json.find(':', catKey);
            if (cc != std::string::npos) {
                size_t cq1 = json.find('\"', cc + 1);
                size_t cq2 = (cq1 != std::string::npos) ? json.find('\"', cq1 + 1) : std::string::npos;
                if (cq1 != std::string::npos && cq2 != std::string::npos)
                    kb.category = json.substr(cq1 + 1, cq2 - cq1 - 1);
            }
        }

        // key
        size_t keyKey = json.find("\"key\"", pos);
        if (keyKey != std::string::npos) {
            size_t kc = json.find(':', keyKey);
            if (kc != std::string::npos) {
                try { kb.key = std::stoi(json.substr(kc + 1)); }
                catch (...) { /* keep default */ }
            }
        }

        // mods
        size_t modKey = json.find("\"mods\"", pos);
        if (modKey != std::string::npos) {
            size_t mc = json.find(':', modKey);
            if (mc != std::string::npos) {
                try { kb.mods = static_cast<KeyMod>(std::stoi(json.substr(mc + 1))); }
                catch (...) { /* keep default */ }
            }
        }

        // enabled
        size_t enKey = json.find("\"enabled\"", pos);
        if (enKey != std::string::npos) {
            size_t ec = json.find(':', enKey);
            if (ec != std::string::npos) {
                size_t vs = json.find_first_not_of(" \t\n\r", ec + 1);
                if (vs != std::string::npos && vs + 4 <= json.size())
                    kb.enabled = (json.substr(vs, 4) == "true");
            }
        }

        if (!kb.action.empty()) {
            loaded.push_back(kb);
        }

        // Move past this block
        pos = json.find('}', pos);
        if (pos == std::string::npos) break;
        ++pos;
    }

    if (loaded.empty()) return false;

    m_bindings = std::move(loaded);
    return true;
}

// ── File I/O ───────────────────────────────────────────────────────

bool KeybindManager::SaveToFile(const std::string& path) const {
    std::filesystem::path fspath(path);
    if (fspath.has_parent_path()) {
        std::filesystem::create_directories(fspath.parent_path());
    }

    std::ofstream out(path);
    if (!out.is_open()) return false;

    out << SerializeToJSON();
    out.close();
    return true;
}

bool KeybindManager::LoadFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) return false;

    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string json((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();

    return DeserializeFromJSON(json);
}

} // namespace atlas::editor
