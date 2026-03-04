// Tests for: Corporation System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "data/world_persistence.h"
#include "systems/corporation_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Corporation System Tests ====================

static void testCorpCreate() {
    std::cout << "\n=== Corporation Create ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* player = world.createEntity("player1");
    auto* pc = addComp<components::Player>(player);
    pc->player_id = "player1";
    pc->character_name = "TestPilot";

    assertTrue(corpSys.createCorporation("player1", "Test Corp", "TSTC"),
               "Corporation created");

    auto* corp_entity = world.getEntity("corp_test_corp");
    assertTrue(corp_entity != nullptr, "Corp entity exists");

    auto* corp = corp_entity->getComponent<components::Corporation>();
    assertTrue(corp != nullptr, "Corporation component exists");
    assertTrue(corp->ceo_id == "player1", "CEO is the creator");
    assertTrue(corp->corp_name == "Test Corp", "Corp name set");
    assertTrue(corp->ticker == "TSTC", "Ticker set");
    assertTrue(corpSys.getMemberCount("corp_test_corp") == 1, "One member after creation");
    assertTrue(pc->corporation == "Test Corp", "Player corporation updated");
}

static void testCorpJoin() {
    std::cout << "\n=== Corporation Join ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Join Corp", "JNCO");

    assertTrue(corpSys.joinCorporation("player2", "corp_join_corp"),
               "Player2 joins corp");
    assertTrue(corpSys.getMemberCount("corp_join_corp") == 2, "Two members after join");
    assertTrue(pc2->corporation == "Join Corp", "Player2 corporation updated");
    assertTrue(!corpSys.joinCorporation("player2", "corp_join_corp"),
               "Duplicate join rejected");
}

static void testCorpLeave() {
    std::cout << "\n=== Corporation Leave ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Leave Corp", "LVCO");
    corpSys.joinCorporation("player2", "corp_leave_corp");

    assertTrue(corpSys.leaveCorporation("player2", "corp_leave_corp"),
               "Player2 leaves corp");
    assertTrue(corpSys.getMemberCount("corp_leave_corp") == 1, "One member after leave");
    assertTrue(pc2->corporation == "NPC Corp", "Player2 corporation reset");
}

static void testCorpCeoCannotLeave() {
    std::cout << "\n=== Corporation CEO Cannot Leave ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    corpSys.createCorporation("player1", "CEO Corp", "CEOC");

    assertTrue(!corpSys.leaveCorporation("player1", "corp_ceo_corp"),
               "CEO cannot leave corporation");
    assertTrue(corpSys.getMemberCount("corp_ceo_corp") == 1, "Member count unchanged");
}

static void testCorpTaxRate() {
    std::cout << "\n=== Corporation Tax Rate ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Tax Corp", "TAXC");
    corpSys.joinCorporation("player2", "corp_tax_corp");

    assertTrue(corpSys.setTaxRate("corp_tax_corp", "player1", 0.10f),
               "CEO can set tax rate");
    auto* corp = world.getEntity("corp_tax_corp")->getComponent<components::Corporation>();
    assertTrue(approxEqual(corp->tax_rate, 0.10f), "Tax rate updated to 10%");

    assertTrue(!corpSys.setTaxRate("corp_tax_corp", "player2", 0.20f),
               "Non-CEO cannot set tax rate");
    assertTrue(approxEqual(corp->tax_rate, 0.10f), "Tax rate unchanged");
}

static void testCorpApplyTax() {
    std::cout << "\n=== Corporation Apply Tax ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    corpSys.createCorporation("player1", "Wallet Corp", "WLTC");
    corpSys.setTaxRate("corp_wallet_corp", "player1", 0.10f);

    double remaining = corpSys.applyTax("corp_wallet_corp", 1000.0);
    assertTrue(approxEqual(static_cast<float>(remaining), 900.0f), "Remaining Credits after 10% tax");

    auto* corp = world.getEntity("corp_wallet_corp")->getComponent<components::Corporation>();
    assertTrue(approxEqual(static_cast<float>(corp->corp_wallet), 100.0f), "Corp wallet received tax");
}

static void testSerializeDeserializeCorporation() {
    std::cout << "\n=== Serialize/Deserialize Corporation ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("corp_test");
    auto* corp = addComp<components::Corporation>(entity);
    corp->corp_id = "corp_test";
    corp->corp_name = "Serialize Corp";
    corp->ticker = "SRLZ";
    corp->ceo_id = "player1";
    corp->tax_rate = 0.15f;
    corp->corp_wallet = 50000.0;
    corp->member_ids.push_back("player1");
    corp->member_ids.push_back("player2");

    components::Corporation::CorpHangarItem item;
    item.item_id = "stellium"; item.name = "Stellium";
    item.type = "ore"; item.quantity = 1000; item.volume = 0.01f;
    corp->hangar_items.push_back(item);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "Corporation deserialization succeeds");

    auto* e2 = world2.getEntity("corp_test");
    assertTrue(e2 != nullptr, "Corp entity recreated");

    auto* corp2 = e2->getComponent<components::Corporation>();
    assertTrue(corp2 != nullptr, "Corporation component recreated");
    assertTrue(corp2->corp_name == "Serialize Corp", "corp_name preserved");
    assertTrue(corp2->ticker == "SRLZ", "ticker preserved");
    assertTrue(corp2->ceo_id == "player1", "ceo_id preserved");
    assertTrue(approxEqual(corp2->tax_rate, 0.15f), "tax_rate preserved");
    assertTrue(approxEqual(static_cast<float>(corp2->corp_wallet), 50000.0f), "corp_wallet preserved");
    assertTrue(corp2->member_ids.size() == 2, "member_ids count preserved");
    assertTrue(corp2->member_ids[0] == "player1", "member_ids[0] preserved");
    assertTrue(corp2->member_ids[1] == "player2", "member_ids[1] preserved");
    assertTrue(corp2->hangar_items.size() == 1, "hangar_items count preserved");
    assertTrue(corp2->hangar_items[0].item_id == "stellium", "hangar item_id preserved");
    assertTrue(corp2->hangar_items[0].quantity == 1000, "hangar item quantity preserved");
}


void run_corporation_system_tests() {
    testCorpCreate();
    testCorpJoin();
    testCorpLeave();
    testCorpCeoCannotLeave();
    testCorpTaxRate();
    testCorpApplyTax();
    testSerializeDeserializeCorporation();
}
