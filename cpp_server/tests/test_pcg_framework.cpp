// Tests for: PCG Framework Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "components/fps_components.h"
#include "components/narrative_components.h"
#include "components/navigation_components.h"
#include "pcg/deterministic_rng.h"
#include "pcg/pcg_context.h"
#include "pcg/pcg_manager.h"
#include "pcg/deck_graph.h"
#include "pcg/room_graph.h"
#include "pcg/ship_designer.h"
#include "pcg/ship_generator.h"
#include "systems/titan_assembly_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== PCG Framework Tests ====================

static void testDeterministicRNGSameSeed() {
    std::cout << "\n=== PCG: DeterministicRNG same-seed determinism ===" << std::endl;
    atlas::pcg::DeterministicRNG a(12345);
    atlas::pcg::DeterministicRNG b(12345);

    bool allMatch = true;
    for (int i = 0; i < 100; ++i) {
        if (a.nextU32() != b.nextU32()) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Two RNGs with same seed produce identical sequence");
}

static void testDeterministicRNGDifferentSeed() {
    std::cout << "\n=== PCG: DeterministicRNG different seeds diverge ===" << std::endl;
    atlas::pcg::DeterministicRNG a(111);
    atlas::pcg::DeterministicRNG b(222);

    bool anyDiffer = false;
    for (int i = 0; i < 20; ++i) {
        if (a.nextU32() != b.nextU32()) { anyDiffer = true; break; }
    }
    assertTrue(anyDiffer, "Different seeds produce different sequences");
}

static void testDeterministicRNGRange() {
    std::cout << "\n=== PCG: DeterministicRNG range bounds ===" << std::endl;
    atlas::pcg::DeterministicRNG rng(42);

    bool inBounds = true;
    for (int i = 0; i < 500; ++i) {
        int v = rng.range(10, 20);
        if (v < 10 || v > 20) { inBounds = false; break; }
    }
    assertTrue(inBounds, "range(10,20) always in [10,20]");

    bool floatOk = true;
    atlas::pcg::DeterministicRNG rng2(99);
    for (int i = 0; i < 500; ++i) {
        float f = rng2.nextFloat();
        if (f < 0.0f || f >= 1.0f) { floatOk = false; break; }
    }
    assertTrue(floatOk, "nextFloat() always in [0,1)");
}

static void testHashCombineDeterminism() {
    std::cout << "\n=== PCG: hashCombine determinism ===" << std::endl;
    uint64_t h1 = atlas::pcg::hashCombine(100, 200);
    uint64_t h2 = atlas::pcg::hashCombine(100, 200);
    assertTrue(h1 == h2, "hashCombine is deterministic");

    uint64_t h3 = atlas::pcg::hashCombine(100, 201);
    assertTrue(h1 != h3, "Different inputs produce different hashes");
}

static void testHash64FourInputs() {
    std::cout << "\n=== PCG: hash64 four-input determinism ===" << std::endl;
    uint64_t a = atlas::pcg::hash64(1, 2, 3, 4);
    uint64_t b = atlas::pcg::hash64(1, 2, 3, 4);
    assertTrue(a == b, "hash64 is deterministic");

    uint64_t c = atlas::pcg::hash64(1, 2, 3, 5);
    assertTrue(a != c, "Changing one input changes hash");
}

static void testDeriveSeed() {
    std::cout << "\n=== PCG: deriveSeed hierarchy ===" << std::endl;
    uint64_t parent = 0xDEADBEEF;
    uint64_t child1 = atlas::pcg::deriveSeed(parent, 1);
    uint64_t child2 = atlas::pcg::deriveSeed(parent, 2);
    uint64_t child1b = atlas::pcg::deriveSeed(parent, 1);

    assertTrue(child1 == child1b, "Same parent+id produces same child seed");
    assertTrue(child1 != child2, "Different ids produce different child seeds");
    assertTrue(child1 != parent, "Child seed differs from parent");
}

static void testPCGManagerInitialize() {
    std::cout << "\n=== PCG: PCGManager initialize ===" << std::endl;
    atlas::pcg::PCGManager mgr;
    assertTrue(!mgr.isInitialized(), "Not initialized before init");

    mgr.initialize(0xCAFEBABE);
    assertTrue(mgr.isInitialized(), "Initialized after init");
    assertTrue(mgr.universeSeed() == 0xCAFEBABE, "Seed stored correctly");
}

static void testPCGManagerContextDeterminism() {
    std::cout << "\n=== PCG: PCGManager context determinism ===" << std::endl;
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);

    auto ctx1 = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 100, 1);
    auto ctx2 = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 100, 1);
    assertTrue(ctx1.seed == ctx2.seed, "Same domain+id → same seed");

    auto ctx3 = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 101, 1);
    assertTrue(ctx1.seed != ctx3.seed, "Different id → different seed");

    auto ctx4 = mgr.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 100, 1);
    assertTrue(ctx1.seed != ctx4.seed, "Different domain → different seed");
}

static void testShipGeneratorDeterminism() {
    std::cout << "\n=== PCG: ShipGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 777, 1 };

    auto ship1 = atlas::pcg::ShipGenerator::generate(ctx);
    auto ship2 = atlas::pcg::ShipGenerator::generate(ctx);

    assertTrue(ship1.hullClass == ship2.hullClass, "Same hull class");
    assertTrue(ship1.mass == ship2.mass, "Same mass");
    assertTrue(ship1.thrust == ship2.thrust, "Same thrust");
    assertTrue(ship1.turretSlots == ship2.turretSlots, "Same turret slots");
    assertTrue(ship1.valid == ship2.valid, "Same validity");
}

static void testShipGeneratorConstraints() {
    std::cout << "\n=== PCG: ShipGenerator constraints ===" << std::endl;
    // Generate many ships and verify invariants.
    bool allValid = true;
    bool thrustOk = true;
    bool massOk = true;

    for (uint64_t i = 1; i <= 200; ++i) {
        atlas::pcg::PCGContext ctx{ i * 137, 1 };
        auto ship = atlas::pcg::ShipGenerator::generate(ctx);
        if (!ship.valid)    allValid = false;
        if (ship.thrust <= 0) thrustOk = false;
        if (ship.mass <= 0)  massOk = false;
    }
    assertTrue(allValid, "All 200 generated ships are valid");
    assertTrue(thrustOk, "All ships have positive thrust");
    assertTrue(massOk, "All ships have positive mass");
}

static void testShipGeneratorHullOverride() {
    std::cout << "\n=== PCG: ShipGenerator hull override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 555, 1 };

    auto frigate = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Frigate);
    assertTrue(frigate.hullClass == atlas::pcg::HullClass::Frigate, "Hull override works (Frigate)");
    assertTrue(frigate.mass >= 1000.0f && frigate.mass <= 2500.0f, "Frigate mass in range");

    auto capital = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Capital);
    assertTrue(capital.hullClass == atlas::pcg::HullClass::Capital, "Hull override works (Capital)");
    assertTrue(capital.mass >= 800000.0f, "Capital mass in range");
    assertTrue(capital.engineCount >= 6, "Capital has ≥ 6 engines");
}

static void testShipGeneratorExpandedFields() {
    std::cout << "\n=== PCG: ShipGenerator expanded fields ===" << std::endl;
    bool armorOk = true;
    bool shieldOk = true;
    bool sigOk = true;
    bool targetOk = true;
    bool nameOk = true;
    bool droneOk = true;

    for (uint64_t i = 1; i <= 100; ++i) {
        atlas::pcg::PCGContext ctx{ i * 43, 1 };
        auto ship = atlas::pcg::ShipGenerator::generate(ctx);
        if (ship.armorHP <= 0.0f) armorOk = false;
        if (ship.shieldHP <= 0.0f) shieldOk = false;
        if (ship.signatureRadius <= 0.0f) sigOk = false;
        if (ship.targetingSpeed <= 0.0f) targetOk = false;
        if (ship.shipName.empty()) nameOk = false;
        if (ship.droneBay < 0) droneOk = false;
    }
    assertTrue(armorOk, "All ships have positive armor");
    assertTrue(shieldOk, "All ships have positive shields");
    assertTrue(sigOk, "All ships have positive signature radius");
    assertTrue(targetOk, "All ships have positive targeting speed");
    assertTrue(nameOk, "All ships have non-empty names");
    assertTrue(droneOk, "All ships have non-negative drone bay");
}

static void testShipGeneratorExpandedDeterminism() {
    std::cout << "\n=== PCG: ShipGenerator expanded determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 777, 1 };
    auto s1 = atlas::pcg::ShipGenerator::generate(ctx);
    auto s2 = atlas::pcg::ShipGenerator::generate(ctx);

    assertTrue(s1.armorHP == s2.armorHP, "Same armor HP");
    assertTrue(s1.shieldHP == s2.shieldHP, "Same shield HP");
    assertTrue(s1.signatureRadius == s2.signatureRadius, "Same signature radius");
    assertTrue(s1.targetingSpeed == s2.targetingSpeed, "Same targeting speed");
    assertTrue(s1.droneBay == s2.droneBay, "Same drone bay");
    assertTrue(s1.shipName == s2.shipName, "Same ship name");
}

static void testShipGeneratorHullRanges() {
    std::cout << "\n=== PCG: ShipGenerator hull-specific ranges ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 999, 1 };

    auto frigate = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Frigate);
    assertTrue(frigate.armorHP >= 300.0f && frigate.armorHP <= 600.0f, "Frigate armor in range");
    assertTrue(frigate.shieldHP >= 250.0f && frigate.shieldHP <= 500.0f, "Frigate shield in range");
    assertTrue(frigate.signatureRadius >= 30.0f && frigate.signatureRadius <= 45.0f, "Frigate sig in range");

    auto bs = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Battleship);
    assertTrue(bs.armorHP >= 6000.0f && bs.armorHP <= 10000.0f, "Battleship armor in range");
    assertTrue(bs.signatureRadius >= 300.0f && bs.signatureRadius <= 450.0f, "Battleship sig in range");

    assertTrue(bs.armorHP > frigate.armorHP, "Battleship armor > Frigate armor");
    assertTrue(bs.signatureRadius > frigate.signatureRadius, "Battleship sig > Frigate sig");
}

static void testShipGeneratorShipName() {
    std::cout << "\n=== PCG: ShipGenerator ship naming ===" << std::endl;
    atlas::pcg::PCGContext ctx1{ 100, 1 };
    atlas::pcg::PCGContext ctx2{ 200, 1 };

    auto s1 = atlas::pcg::ShipGenerator::generate(ctx1);
    auto s2 = atlas::pcg::ShipGenerator::generate(ctx2);

    assertTrue(!s1.shipName.empty(), "Ship 1 has a name");
    assertTrue(!s2.shipName.empty(), "Ship 2 has a name");
    // Names from different seeds should usually differ.
    assertTrue(s1.shipName != s2.shipName, "Different seeds produce different names");
    // Name contains a hyphen (format: Prefix Suffix-Serial).
    assertTrue(s1.shipName.find('-') != std::string::npos, "Ship name contains serial separator");
}

static void testRoomGraphFunctionalTypes() {
    std::cout << "\n=== PCG: RoomGraph functional type assignment ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 42, 1 };

    // Deck 0: first room should be Cockpit.
    auto deck0 = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    assertTrue(deck0[0].type == atlas::pcg::RoomType::Cockpit,
               "Deck 0 first room is Cockpit");

    // Last room should be Engine.
    assertTrue(deck0.back().type == atlas::pcg::RoomType::Engine,
               "Last room on deck 0 is Engine");

    // Deck 1+: first room should be Reactor.
    auto deck1 = atlas::pcg::generateRoomsForDeck(1, ctx, 3);
    assertTrue(deck1[0].type == atlas::pcg::RoomType::Reactor,
               "Deck 1 first room is Reactor");
    assertTrue(deck1.back().type == atlas::pcg::RoomType::Engine,
               "Last room on deck 1 is Engine");
}

static void testRoomGraphDimensionsByType() {
    std::cout << "\n=== PCG: RoomGraph dimensions vary by type ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 50, 1 };

    // Generate deck 0 with enough rooms to include varied types.
    auto rooms = atlas::pcg::generateRoomsForDeck(0, ctx, 5);

    for (const auto& r : rooms) {
        switch (r.type) {
            case atlas::pcg::RoomType::Engine:
                assertTrue(r.dimZ >= 6.0f, "Engine room Z >= 6m");
                break;
            case atlas::pcg::RoomType::Corridor:
                assertTrue(r.dimX <= 3.0f, "Corridor X <= 3m");
                break;
            case atlas::pcg::RoomType::Reactor:
                assertTrue(r.dimX >= 5.0f, "Reactor X >= 5m");
                break;
            default:
                assertTrue(r.dimX > 0.0f && r.dimY > 0.0f && r.dimZ > 0.0f,
                           "Room has positive dimensions");
                break;
        }
    }
}

static void testDeckGraphHubAndSpoke() {
    std::cout << "\n=== PCG: DeckGraph hub-and-spoke corridors ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 200, 1 };

    // Use shipClass 4 to get enough rooms for hub-and-spoke to kick in.
    auto decks = atlas::pcg::generateDeckStack(4, ctx);
    bool hubFound = false;
    for (const auto& deck : decks) {
        if (deck.rooms.size() >= 4) {
            // Should have linear (N-1) + hub ((N-2)) corridors.
            size_t expected = (deck.rooms.size() - 1) + (deck.rooms.size() - 2);
            assertTrue(deck.corridors.size() == expected,
                       "Deck has linear + hub-and-spoke corridors");
            hubFound = true;
            break;
        }
    }
    assertTrue(hubFound, "At least one deck has hub-and-spoke connections");
}

static void testShipDesignerSaveRoundTrip() {
    std::cout << "\n=== PCG: ShipDesigner save captures rooms ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 800, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);

    // Count total rooms.
    int totalRooms = 0;
    for (const auto& d : decks) totalRooms += static_cast<int>(d.rooms.size());

    auto save = atlas::pcg::saveShipLayout(decks, 2, 800);
    assertTrue(save.pcgVersion == 1, "Save version set");
    assertTrue(save.shipClass == 2, "Save class stored");
    assertTrue(save.seed == 800, "Save seed stored");
    assertTrue(static_cast<int>(save.roomOverrides.size()) == totalRooms,
               "Save captures all rooms as overrides");
}

static void testTitanAssemblyDefaults() {
    std::cout << "\n=== Systems: TitanAssembly defaults ===" << std::endl;
    atlas::systems::TitanAssemblyComponent comp;
    assertTrue(comp.progress == 0.0f, "Initial progress is 0");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Rumor, "Initial phase is Rumor");
    assertTrue(!comp.disrupted, "Not disrupted initially");
    assertTrue(comp.disruptCount == 0, "No disruptions initially");
}

static void testTitanAssemblyTick() {
    std::cout << "\n=== Systems: TitanAssembly tick progression ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.resourceRate = 0.05f;

    // 4 ticks → progress = 0.20
    for (int i = 0; i < 4; ++i) sys.tick(comp);
    assertTrue(comp.progress >= 0.19f && comp.progress <= 0.21f, "4 ticks at 0.05 ≈ 0.20");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Unease, "Phase transitions to Unease at 20%");

    // 6 more ticks → progress = 0.50
    for (int i = 0; i < 6; ++i) sys.tick(comp);
    assertTrue(comp.progress >= 0.49f && comp.progress <= 0.51f, "10 ticks at 0.05 ≈ 0.50");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Fear, "Phase transitions to Fear at 50%");
}

static void testTitanAssemblyDisrupt() {
    std::cout << "\n=== Systems: TitanAssembly disruption ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.progress = 0.50f;
    comp.phase = atlas::systems::TitanPhase::Fear;

    sys.disrupt(comp, 0.35f);
    assertTrue(comp.progress >= 0.14f && comp.progress <= 0.16f, "Progress reduced by disruption");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Rumor, "Phase regressed to Rumor");
    assertTrue(comp.disruptCount == 1, "Disruption counted");
    assertTrue(comp.disrupted, "Disrupted flag set");
}

static void testTitanAssemblyDisruptedTick() {
    std::cout << "\n=== Systems: TitanAssembly disrupted tick ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.resourceRate = 0.10f;
    comp.disrupted = true;

    sys.tick(comp);
    // Disrupted tick advances at 25% rate: 0.10 * 0.25 = 0.025
    assertTrue(comp.progress >= 0.024f && comp.progress <= 0.026f, "Disrupted tick at 25% rate");
    assertTrue(!comp.disrupted, "Disrupted flag cleared after tick");
}

static void testTitanAssemblyClamped() {
    std::cout << "\n=== Systems: TitanAssembly progress clamped ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.progress = 0.99f;
    comp.resourceRate = 0.10f;

    sys.tick(comp);
    assertTrue(comp.progress == 1.0f, "Progress clamped at 1.0");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Acceptance, "Acceptance phase at 100%");
}

static void testTitanAssemblyDisruptFloor() {
    std::cout << "\n=== Systems: TitanAssembly disrupt floor ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.progress = 0.05f;

    sys.disrupt(comp, 0.50f);
    assertTrue(comp.progress == 0.0f, "Progress floored at 0.0");
}

static void testTitanAssemblyPhaseName() {
    std::cout << "\n=== Systems: TitanAssembly phase names ===" << std::endl;
    using atlas::systems::TitanPhase;
    using atlas::systems::TitanAssemblySystem;
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Rumor) == "Rumor", "Rumor name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Unease) == "Unease", "Unease name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Fear) == "Fear", "Fear name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Acceptance) == "Acceptance", "Acceptance name");
}


void run_pcg_framework_tests() {
    testDeterministicRNGSameSeed();
    testDeterministicRNGDifferentSeed();
    testDeterministicRNGRange();
    testHashCombineDeterminism();
    testHash64FourInputs();
    testDeriveSeed();
    testPCGManagerInitialize();
    testPCGManagerContextDeterminism();
    testShipGeneratorDeterminism();
    testShipGeneratorConstraints();
    testShipGeneratorHullOverride();
    testShipGeneratorExpandedFields();
    testShipGeneratorExpandedDeterminism();
    testShipGeneratorHullRanges();
    testShipGeneratorShipName();
    testRoomGraphFunctionalTypes();
    testRoomGraphDimensionsByType();
    testDeckGraphHubAndSpoke();
    testShipDesignerSaveRoundTrip();
    testTitanAssemblyDefaults();
    testTitanAssemblyTick();
    testTitanAssemblyDisrupt();
    testTitanAssemblyDisruptedTick();
    testTitanAssemblyClamped();
    testTitanAssemblyDisruptFloor();
    testTitanAssemblyPhaseName();
}
