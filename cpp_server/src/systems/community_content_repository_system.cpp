#include "systems/community_content_repository_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::CommunityContentRepo::ContentEntry* findContent(
    components::CommunityContentRepo* repo, const std::string& content_id) {
    for (auto& entry : repo->contents) {
        if (entry.content_id == content_id) return &entry;
    }
    return nullptr;
}
} // anonymous namespace

CommunityContentRepositorySystem::CommunityContentRepositorySystem(ecs::World* world)
    : System(world) {
}

void CommunityContentRepositorySystem::update(float delta_time) {
    auto entities = world_->getEntities<components::CommunityContentRepo>();
    for (auto* entity : entities) {
        auto* repo = entity->getComponent<components::CommunityContentRepo>();
        if (!repo || !repo->active) continue;
        // Periodic maintenance — no time-based logic required
    }
}

bool CommunityContentRepositorySystem::createRepo(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CommunityContentRepo>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CommunityContentRepositorySystem::submitContent(
    const std::string& entity_id, const std::string& content_id,
    const std::string& type, const std::string& author,
    const std::string& title, const std::string& description) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return false;
    if (static_cast<int>(repo->contents.size()) >= repo->max_content) return false;
    // Check for duplicate content_id
    if (findContent(repo, content_id)) return false;

    components::CommunityContentRepo::ContentEntry entry;
    entry.content_id = content_id;
    entry.type = type;
    entry.author = author;
    entry.title = title;
    entry.description = description;
    entry.state = "Draft";
    repo->contents.push_back(entry);
    repo->total_submissions++;
    return true;
}

bool CommunityContentRepositorySystem::publishContent(
    const std::string& entity_id, const std::string& content_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return false;
    auto* entry = findContent(repo, content_id);
    if (!entry || entry->state != "Draft") return false;
    entry->state = "Published";
    return true;
}

bool CommunityContentRepositorySystem::featureContent(
    const std::string& entity_id, const std::string& content_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return false;
    auto* entry = findContent(repo, content_id);
    if (!entry || entry->state != "Published") return false;
    entry->state = "Featured";
    return true;
}

bool CommunityContentRepositorySystem::archiveContent(
    const std::string& entity_id, const std::string& content_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return false;
    auto* entry = findContent(repo, content_id);
    if (!entry) return false;
    entry->state = "Archived";
    return true;
}

bool CommunityContentRepositorySystem::rateContent(
    const std::string& entity_id, const std::string& content_id, int rating) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return false;
    auto* entry = findContent(repo, content_id);
    if (!entry) return false;
    rating = std::max(1, std::min(rating, 5));
    entry->total_rating += rating;
    entry->rating_count++;
    entry->average_rating = static_cast<float>(entry->total_rating) /
                            static_cast<float>(entry->rating_count);
    return true;
}

bool CommunityContentRepositorySystem::downloadContent(
    const std::string& entity_id, const std::string& content_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return false;
    auto* entry = findContent(repo, content_id);
    if (!entry) return false;
    entry->downloads++;
    repo->total_downloads++;
    return true;
}

int CommunityContentRepositorySystem::getContentCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return 0;
    return static_cast<int>(repo->contents.size());
}

float CommunityContentRepositorySystem::getAverageRating(
    const std::string& entity_id, const std::string& content_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return 0.0f;
    for (const auto& entry : repo->contents) {
        if (entry.content_id == content_id) return entry.average_rating;
    }
    return 0.0f;
}

int CommunityContentRepositorySystem::getDownloadCount(
    const std::string& entity_id, const std::string& content_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return 0;
    for (const auto& entry : repo->contents) {
        if (entry.content_id == content_id) return entry.downloads;
    }
    return 0;
}

int CommunityContentRepositorySystem::getTotalDownloads(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return 0;
    return repo->total_downloads;
}

int CommunityContentRepositorySystem::getContentByType(
    const std::string& entity_id, const std::string& type) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return 0;
    int count = 0;
    for (const auto& entry : repo->contents) {
        if (entry.type == type) count++;
    }
    return count;
}

std::string CommunityContentRepositorySystem::getContentState(
    const std::string& entity_id, const std::string& content_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return "";
    for (const auto& entry : repo->contents) {
        if (entry.content_id == content_id) return entry.state;
    }
    return "";
}

int CommunityContentRepositorySystem::getTotalSubmissions(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* repo = entity->getComponent<components::CommunityContentRepo>();
    if (!repo) return 0;
    return repo->total_submissions;
}

} // namespace systems
} // namespace atlas
