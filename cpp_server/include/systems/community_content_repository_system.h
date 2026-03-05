#ifndef NOVAFORGE_SYSTEMS_COMMUNITY_CONTENT_REPOSITORY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMMUNITY_CONTENT_REPOSITORY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Community content sharing repository for user-created game content
 *
 * Manages submission, publishing, rating, and download tracking of
 * community-created content such as ships, modules, missions, and skins.
 */
class CommunityContentRepositorySystem : public ecs::SingleComponentSystem<components::CommunityContentRepo> {
public:
    explicit CommunityContentRepositorySystem(ecs::World* world);
    ~CommunityContentRepositorySystem() override = default;

    std::string getName() const override { return "CommunityContentRepositorySystem"; }

    bool createRepo(const std::string& entity_id);
    bool submitContent(const std::string& entity_id, const std::string& content_id,
                       const std::string& type, const std::string& author,
                       const std::string& title, const std::string& description);
    bool publishContent(const std::string& entity_id, const std::string& content_id);
    bool featureContent(const std::string& entity_id, const std::string& content_id);
    bool archiveContent(const std::string& entity_id, const std::string& content_id);
    bool rateContent(const std::string& entity_id, const std::string& content_id, int rating);
    bool downloadContent(const std::string& entity_id, const std::string& content_id);
    int getContentCount(const std::string& entity_id) const;
    float getAverageRating(const std::string& entity_id, const std::string& content_id) const;
    int getDownloadCount(const std::string& entity_id, const std::string& content_id) const;
    int getTotalDownloads(const std::string& entity_id) const;
    int getContentByType(const std::string& entity_id, const std::string& type) const;
    std::string getContentState(const std::string& entity_id, const std::string& content_id) const;
    int getTotalSubmissions(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CommunityContentRepo& repo, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMMUNITY_CONTENT_REPOSITORY_SYSTEM_H
