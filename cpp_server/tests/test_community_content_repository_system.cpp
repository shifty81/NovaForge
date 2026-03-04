// Tests for: CommunityContentRepositorySystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/community_content_repository_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== CommunityContentRepositorySystem Tests ====================

static void testCommunityContentCreate() {
    std::cout << "\n=== CommunityContentRepository: Create ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    assertTrue(sys.createRepo("repo1"), "Create repo succeeds");
    assertTrue(sys.getContentCount("repo1") == 0, "Initial content count is 0");
    assertTrue(sys.getTotalSubmissions("repo1") == 0, "Initial total submissions is 0");
    assertTrue(sys.getTotalDownloads("repo1") == 0, "Initial total downloads is 0");
}

static void testCommunityContentSubmit() {
    std::cout << "\n=== CommunityContentRepository: Submit ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    assertTrue(sys.submitContent("repo1", "ship1", "Ship", "alice", "Cool Ship", "A neat ship"), "Submit ship succeeds");
    assertTrue(sys.getContentCount("repo1") == 1, "Content count is 1 after submit");
    assertTrue(sys.getTotalSubmissions("repo1") == 1, "Total submissions is 1");
    assertTrue(sys.getContentState("repo1", "ship1") == "Draft", "New content starts as Draft");
}

static void testCommunityContentPublish() {
    std::cout << "\n=== CommunityContentRepository: Publish ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "ship1", "Ship", "alice", "Cool Ship", "A neat ship");
    assertTrue(sys.publishContent("repo1", "ship1"), "Publish Draft content succeeds");
    assertTrue(sys.getContentState("repo1", "ship1") == "Published", "State is Published");
    assertTrue(!sys.publishContent("repo1", "ship1"), "Cannot publish already Published content");
}

static void testCommunityContentFeature() {
    std::cout << "\n=== CommunityContentRepository: Feature ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "mod1", "Module", "bob", "Mod A", "desc");
    assertTrue(!sys.featureContent("repo1", "mod1"), "Cannot feature Draft content");
    sys.publishContent("repo1", "mod1");
    assertTrue(sys.featureContent("repo1", "mod1"), "Feature Published content succeeds");
    assertTrue(sys.getContentState("repo1", "mod1") == "Featured", "State is Featured");
}

static void testCommunityContentArchive() {
    std::cout << "\n=== CommunityContentRepository: Archive ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "skin1", "Skin", "carol", "Skin A", "desc");
    assertTrue(sys.archiveContent("repo1", "skin1"), "Archive Draft content succeeds");
    assertTrue(sys.getContentState("repo1", "skin1") == "Archived", "State is Archived");
}

static void testCommunityContentRate() {
    std::cout << "\n=== CommunityContentRepository: Rate ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "m1", "Mission", "dave", "Mission A", "desc");
    assertTrue(sys.rateContent("repo1", "m1", 5), "Rate 5 stars succeeds");
    assertTrue(approxEqual(sys.getAverageRating("repo1", "m1"), 5.0f), "Average rating is 5.0");
    assertTrue(sys.rateContent("repo1", "m1", 3), "Rate 3 stars succeeds");
    assertTrue(approxEqual(sys.getAverageRating("repo1", "m1"), 4.0f), "Average rating is 4.0 after two ratings");
    assertTrue(sys.rateContent("repo1", "m1", 1), "Rate 1 star succeeds");
    assertTrue(approxEqual(sys.getAverageRating("repo1", "m1"), 3.0f), "Average rating is 3.0 after three ratings");
}

static void testCommunityContentDownload() {
    std::cout << "\n=== CommunityContentRepository: Download ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "ship1", "Ship", "alice", "Cool Ship", "desc");
    assertTrue(sys.downloadContent("repo1", "ship1"), "Download succeeds");
    assertTrue(sys.getDownloadCount("repo1", "ship1") == 1, "Download count is 1");
    sys.downloadContent("repo1", "ship1");
    sys.downloadContent("repo1", "ship1");
    assertTrue(sys.getDownloadCount("repo1", "ship1") == 3, "Download count is 3 after 3 downloads");
    assertTrue(sys.getTotalDownloads("repo1") == 3, "Total downloads is 3");
}

static void testCommunityContentByType() {
    std::cout << "\n=== CommunityContentRepository: ByType ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "s1", "Ship", "a", "S1", "d");
    sys.submitContent("repo1", "s2", "Ship", "b", "S2", "d");
    sys.submitContent("repo1", "m1", "Module", "c", "M1", "d");
    assertTrue(sys.getContentByType("repo1", "Ship") == 2, "2 Ship entries");
    assertTrue(sys.getContentByType("repo1", "Module") == 1, "1 Module entry");
    assertTrue(sys.getContentByType("repo1", "Mission") == 0, "0 Mission entries");
}

static void testCommunityContentDuplicate() {
    std::cout << "\n=== CommunityContentRepository: Duplicate ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    assertTrue(sys.submitContent("repo1", "dup1", "Ship", "a", "T", "D"), "First submit succeeds");
    assertTrue(!sys.submitContent("repo1", "dup1", "Ship", "b", "T2", "D2"), "Duplicate id rejected");
    assertTrue(sys.getContentCount("repo1") == 1, "Still 1 content after dup attempt");
}

static void testCommunityContentWorkflow() {
    std::cout << "\n=== CommunityContentRepository: Workflow ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "w1", "Skin", "a", "Skin W", "desc");
    assertTrue(sys.getContentState("repo1", "w1") == "Draft", "Starts Draft");
    sys.publishContent("repo1", "w1");
    assertTrue(sys.getContentState("repo1", "w1") == "Published", "Transitions to Published");
    sys.featureContent("repo1", "w1");
    assertTrue(sys.getContentState("repo1", "w1") == "Featured", "Transitions to Featured");
    sys.archiveContent("repo1", "w1");
    assertTrue(sys.getContentState("repo1", "w1") == "Archived", "Transitions to Archived");
    assertTrue(sys.getTotalSubmissions("repo1") == 1, "Total submissions remains 1");
}

static void testCommunityContentMissing() {
    std::cout << "\n=== CommunityContentRepository: Missing ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    assertTrue(!sys.createRepo("nonexistent"), "Create fails on missing entity");
    assertTrue(!sys.submitContent("nonexistent", "x", "Ship", "a", "T", "D"), "Submit fails on missing");
    assertTrue(!sys.publishContent("nonexistent", "x"), "Publish fails on missing");
    assertTrue(!sys.featureContent("nonexistent", "x"), "Feature fails on missing");
    assertTrue(!sys.archiveContent("nonexistent", "x"), "Archive fails on missing");
    assertTrue(!sys.rateContent("nonexistent", "x", 5), "Rate fails on missing");
    assertTrue(!sys.downloadContent("nonexistent", "x"), "Download fails on missing");
    assertTrue(sys.getContentCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getTotalDownloads("nonexistent") == 0, "0 downloads on missing");
    assertTrue(sys.getContentState("nonexistent", "x") == "", "Empty state on missing");
}


void run_community_content_repository_system_tests() {
    testCommunityContentCreate();
    testCommunityContentSubmit();
    testCommunityContentPublish();
    testCommunityContentFeature();
    testCommunityContentArchive();
    testCommunityContentRate();
    testCommunityContentDownload();
    testCommunityContentByType();
    testCommunityContentDuplicate();
    testCommunityContentWorkflow();
    testCommunityContentMissing();
}
