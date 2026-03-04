// Tests for: Expanded Ship Class Tests
#include "test_log.h"
#include "components/core_components.h"
#include "pcg/pcg_context.h"
#include "pcg/ship_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Expanded Ship Class Tests ====================

static void testShipGeneratorAllHullClasses() {
    std::cout << "\n=== PCG: ShipGenerator all 20 hull classes ===" << std::endl;
    using atlas::pcg::HullClass;
    using atlas::pcg::ShipGenerator;
    using atlas::pcg::PCGContext;

    HullClass allClasses[] = {
        HullClass::Frigate, HullClass::Destroyer, HullClass::Cruiser,
        HullClass::Battlecruiser, HullClass::Battleship, HullClass::Capital,
        HullClass::Interceptor, HullClass::CovertOps, HullClass::AssaultFrigate,
        HullClass::StealthBomber, HullClass::Logistics, HullClass::Recon,
        HullClass::CommandShip, HullClass::Marauder, HullClass::Industrial,
        HullClass::MiningBarge, HullClass::Exhumer, HullClass::Carrier,
        HullClass::Dreadnought, HullClass::Titan,
    };

    for (int i = 0; i < 20; ++i) {
        PCGContext ctx{ static_cast<uint64_t>(i * 1337 + 42), 1 };
        auto ship = ShipGenerator::generate(ctx, allClasses[i]);
        assertTrue(ship.valid, ("Valid ship for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
        assertTrue(ship.hullClass == allClasses[i],
                   ("Hull class matches for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
        assertTrue(ship.mass > 0.0f, ("Positive mass for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
        assertTrue(ship.thrust > 0.0f, ("Positive thrust for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
    }
}

static void testShipGeneratorTechLevels() {
    std::cout << "\n=== PCG: ShipGenerator tech levels ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 555, 1 };
    auto frigate = ShipGenerator::generate(ctx, HullClass::Frigate);
    assertTrue(frigate.techLevel == 1, "Frigate is Tech I");

    auto interceptor = ShipGenerator::generate(ctx, HullClass::Interceptor);
    assertTrue(interceptor.techLevel == 2, "Interceptor is Tech II");

    auto marauder = ShipGenerator::generate(ctx, HullClass::Marauder);
    assertTrue(marauder.techLevel == 2, "Marauder is Tech II");

    auto industrial = ShipGenerator::generate(ctx, HullClass::Industrial);
    assertTrue(industrial.techLevel == 1, "Industrial is Tech I");

    auto exhumer = ShipGenerator::generate(ctx, HullClass::Exhumer);
    assertTrue(exhumer.techLevel == 2, "Exhumer is Tech II");

    auto titan = ShipGenerator::generate(ctx, HullClass::Titan);
    assertTrue(titan.techLevel == 1, "Titan is Tech I");
}

static void testShipGeneratorCargoCapacity() {
    std::cout << "\n=== PCG: ShipGenerator cargo capacity ===" << std::endl;
    using namespace atlas::pcg;

    // Industrials should have much larger cargo than combat ships.
    float industrialCargoSum = 0.0f;
    float frigateCargoSum = 0.0f;
    for (int i = 0; i < 20; ++i) {
        PCGContext ctx{ static_cast<uint64_t>(i * 99 + 7), 1 };
        auto ind = ShipGenerator::generate(ctx, HullClass::Industrial);
        industrialCargoSum += static_cast<float>(ind.cargoCapacity);
        auto frig = ShipGenerator::generate(ctx, HullClass::Frigate);
        frigateCargoSum += static_cast<float>(frig.cargoCapacity);
    }
    assertTrue(industrialCargoSum > frigateCargoSum * 5.0f,
               "Industrials have much larger cargo than frigates");
}

static void testShipGeneratorXLargeWeapons() {
    std::cout << "\n=== PCG: ShipGenerator XLarge weapons ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 777, 1 };
    auto dread = ShipGenerator::generate(ctx, HullClass::Dreadnought);
    assertTrue(dread.maxWeaponSize == WeaponSize::XLarge, "Dreadnought has XLarge weapons");

    auto titan = ShipGenerator::generate(ctx, HullClass::Titan);
    assertTrue(titan.maxWeaponSize == WeaponSize::XLarge, "Titan has XLarge weapons");

    auto frigate = ShipGenerator::generate(ctx, HullClass::Frigate);
    assertTrue(frigate.maxWeaponSize == WeaponSize::Small, "Frigate has Small weapons");
}

static void testShipGeneratorStealthBomberLaunchers() {
    std::cout << "\n=== PCG: ShipGenerator stealth bomber launchers ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 888, 1 };
    auto bomber = ShipGenerator::generate(ctx, HullClass::StealthBomber);
    assertTrue(bomber.launcherSlots >= 3, "StealthBomber has 3+ launcher slots");
    assertTrue(bomber.maxWeaponSize == WeaponSize::Large, "StealthBomber uses Large launchers");
}

static void testShipGeneratorCarrierNoDPS() {
    std::cout << "\n=== PCG: ShipGenerator carrier has no turrets ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 999, 1 };
    auto carrier = ShipGenerator::generate(ctx, HullClass::Carrier);
    assertTrue(carrier.turretSlots == 0, "Carrier has 0 turret slots");
    assertTrue(carrier.launcherSlots == 0, "Carrier has 0 launcher slots");
    assertTrue(carrier.droneBay >= 200, "Carrier has large drone bay (200+)");
}

static void testShipGeneratorHullClassNames() {
    std::cout << "\n=== PCG: ShipGenerator all hull class names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(ShipGenerator::hullClassName(HullClass::Interceptor) == "Interceptor", "Interceptor name");
    assertTrue(ShipGenerator::hullClassName(HullClass::CovertOps) == "CovertOps", "CovertOps name");
    assertTrue(ShipGenerator::hullClassName(HullClass::AssaultFrigate) == "AssaultFrigate", "AssaultFrigate name");
    assertTrue(ShipGenerator::hullClassName(HullClass::StealthBomber) == "StealthBomber", "StealthBomber name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Logistics) == "Logistics", "Logistics name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Recon) == "Recon", "Recon name");
    assertTrue(ShipGenerator::hullClassName(HullClass::CommandShip) == "CommandShip", "CommandShip name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Marauder) == "Marauder", "Marauder name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Industrial) == "Industrial", "Industrial name");
    assertTrue(ShipGenerator::hullClassName(HullClass::MiningBarge) == "MiningBarge", "MiningBarge name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Exhumer) == "Exhumer", "Exhumer name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Carrier) == "Carrier", "Carrier name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Dreadnought) == "Dreadnought", "Dreadnought name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Titan) == "Titan", "Titan name");
}

static void testBaseHullClassMapping() {
    std::cout << "\n=== PCG: baseHullClass mapping ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(baseHullClass(HullClass::Interceptor) == HullClass::Frigate, "Interceptor base is Frigate");
    assertTrue(baseHullClass(HullClass::CovertOps) == HullClass::Frigate, "CovertOps base is Frigate");
    assertTrue(baseHullClass(HullClass::AssaultFrigate) == HullClass::Frigate, "AssaultFrigate base is Frigate");
    assertTrue(baseHullClass(HullClass::StealthBomber) == HullClass::Frigate, "StealthBomber base is Frigate");
    assertTrue(baseHullClass(HullClass::Logistics) == HullClass::Cruiser, "Logistics base is Cruiser");
    assertTrue(baseHullClass(HullClass::Recon) == HullClass::Cruiser, "Recon base is Cruiser");
    assertTrue(baseHullClass(HullClass::CommandShip) == HullClass::Battlecruiser, "CommandShip base is Battlecruiser");
    assertTrue(baseHullClass(HullClass::Marauder) == HullClass::Battleship, "Marauder base is Battleship");
    assertTrue(baseHullClass(HullClass::Industrial) == HullClass::Cruiser, "Industrial base is Cruiser");
    assertTrue(baseHullClass(HullClass::Carrier) == HullClass::Capital, "Carrier base is Capital");
    assertTrue(baseHullClass(HullClass::Dreadnought) == HullClass::Capital, "Dreadnought base is Capital");
    assertTrue(baseHullClass(HullClass::Titan) == HullClass::Capital, "Titan base is Capital");
    assertTrue(baseHullClass(HullClass::Frigate) == HullClass::Frigate, "Frigate base is Frigate");
}


void run_expanded_ship_class_tests() {
    testShipGeneratorAllHullClasses();
    testShipGeneratorTechLevels();
    testShipGeneratorCargoCapacity();
    testShipGeneratorXLargeWeapons();
    testShipGeneratorStealthBomberLaunchers();
    testShipGeneratorCarrierNoDPS();
    testShipGeneratorHullClassNames();
    testBaseHullClassMapping();
}
