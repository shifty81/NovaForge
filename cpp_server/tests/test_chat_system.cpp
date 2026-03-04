// Tests for: Chat System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/chat_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Chat System Tests ====================

static void testChatJoinChannel() {
    std::cout << "\n=== Chat Join Channel ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->channel_name = "local";

    assertTrue(chatSys.joinChannel("chat_channel_1", "player_1", "Alice"),
               "Player 1 joins channel");
    assertTrue(chatSys.joinChannel("chat_channel_1", "player_2", "Bob"),
               "Player 2 joins channel");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 2,
               "Member count is 2");
    // 2 join system messages
    assertTrue(chatSys.getMessageCount("chat_channel_1") >= 2,
               "System join messages sent");
}

static void testChatLeaveChannel() {
    std::cout << "\n=== Chat Leave Channel ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 1,
               "Member count is 1 after join");

    assertTrue(chatSys.leaveChannel("chat_channel_1", "player_1"),
               "Player leaves channel");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 0,
               "Member count is 0 after leave");
    // 1 join + 1 leave system message
    bool hasLeaveMsg = false;
    auto* ch = entity->getComponent<components::ChatChannel>();
    for (const auto& m : ch->messages) {
        if (m.content.find("has left the channel") != std::string::npos)
            hasLeaveMsg = true;
    }
    assertTrue(hasLeaveMsg, "Leave system message exists");
}

static void testChatSendMessage() {
    std::cout << "\n=== Chat Send Message ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    int baseCount = chatSys.getMessageCount("chat_channel_1");

    assertTrue(chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "Hello!"),
               "First message sent");
    assertTrue(chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "World!"),
               "Second message sent");
    assertTrue(chatSys.getMessageCount("chat_channel_1") == baseCount + 2,
               "Message count increased by 2");
}

static void testChatMutePlayer() {
    std::cout << "\n=== Chat Mute Player ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_mod", "Moderator");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    // Set moderator role
    for (auto& m : channel->members) {
        if (m.player_id == "player_mod") m.role = "moderator";
    }

    assertTrue(chatSys.mutePlayer("chat_channel_1", "player_mod", "player_2"),
               "Moderator mutes player");
    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "test"),
               "Muted player cannot send message");
}

static void testChatUnmutePlayer() {
    std::cout << "\n=== Chat Unmute Player ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_mod", "Moderator");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    for (auto& m : channel->members) {
        if (m.player_id == "player_mod") m.role = "moderator";
    }

    chatSys.mutePlayer("chat_channel_1", "player_mod", "player_2");
    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "blocked"),
               "Muted player cannot send");

    assertTrue(chatSys.unmutePlayer("chat_channel_1", "player_mod", "player_2"),
               "Moderator unmutes player");
    assertTrue(chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "free!"),
               "Unmuted player can send again");
}

static void testChatSetMotd() {
    std::cout << "\n=== Chat Set MOTD ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_owner", "Owner");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    // Set owner role
    for (auto& m : channel->members) {
        if (m.player_id == "player_owner") m.role = "owner";
    }

    assertTrue(chatSys.setMotd("chat_channel_1", "player_owner", "Welcome!"),
               "Owner sets MOTD");
    assertTrue(channel->motd == "Welcome!", "MOTD was set correctly");

    assertTrue(!chatSys.setMotd("chat_channel_1", "player_2", "Hacked!"),
               "Regular member cannot set MOTD");
    assertTrue(channel->motd == "Welcome!", "MOTD unchanged after failed attempt");
}

static void testChatMaxMembers() {
    std::cout << "\n=== Chat Max Members ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->max_members = 2;

    assertTrue(chatSys.joinChannel("chat_channel_1", "player_1", "Alice"),
               "Player 1 joins (1/2)");
    assertTrue(chatSys.joinChannel("chat_channel_1", "player_2", "Bob"),
               "Player 2 joins (2/2)");
    assertTrue(!chatSys.joinChannel("chat_channel_1", "player_3", "Charlie"),
               "Player 3 cannot join (channel full)");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 2,
               "Member count stays at 2");
}

static void testChatMessageHistory() {
    std::cout << "\n=== Chat Message History ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->max_history = 5;

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    // join message = 1, then send 8 more = 9 total
    for (int i = 0; i < 8; ++i) {
        chatSys.sendMessage("chat_channel_1", "player_1", "Alice",
                            "Message " + std::to_string(i));
    }
    assertTrue(static_cast<int>(channel->messages.size()) > 5,
               "Messages exceed max_history before trim");

    chatSys.update(0.0f);
    assertTrue(static_cast<int>(channel->messages.size()) <= 5,
               "Messages trimmed to max_history after update");
}

static void testChatMutedPlayerCannotSend() {
    std::cout << "\n=== Chat Muted Player Cannot Send ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");

    // Directly mute via component
    for (auto& m : channel->members) {
        if (m.player_id == "player_1") m.is_muted = true;
    }

    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "test"),
               "Directly muted player cannot send");
}

static void testChatNonMemberCannotSend() {
    std::cout << "\n=== Chat Non-Member Cannot Send ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "test"),
               "Non-member cannot send message");
}


void run_chat_system_tests() {
    testChatJoinChannel();
    testChatLeaveChannel();
    testChatSendMessage();
    testChatMutePlayer();
    testChatUnmutePlayer();
    testChatSetMotd();
    testChatMaxMembers();
    testChatMessageHistory();
    testChatMutedPlayerCannotSend();
    testChatNonMemberCannotSend();
}
