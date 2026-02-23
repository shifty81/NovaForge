#include "pcg/interior_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Room dimension ranges per type ──────────────────────────────────

struct RoomSizeRange {
    float minDim;   // minimum side length (metres)
    float maxDim;   // maximum side length
    float heightMin;
    float heightMax;
};

static RoomSizeRange sizeForType(InteriorRoomType type) {
    switch (type) {
        case InteriorRoomType::Cockpit:
        case InteriorRoomType::Bridge:
        case InteriorRoomType::CICNode:
            return { 2.0f, 6.0f, 2.5f, 3.5f };
        case InteriorRoomType::Engineering:
        case InteriorRoomType::Reactor:
            return { 4.0f, 10.0f, 3.0f, 5.0f };
        case InteriorRoomType::Hangar:
        case InteriorRoomType::CargoBay:
            return { 6.0f, 16.0f, 4.0f, 8.0f };
        case InteriorRoomType::WeaponControl:
        case InteriorRoomType::Magazine:
            return { 3.0f, 6.0f, 2.5f, 3.0f };
        case InteriorRoomType::CrewQuarters:
        case InteriorRoomType::Barracks:
        case InteriorRoomType::Medbay:
        case InteriorRoomType::MessHall:
            return { 3.0f, 7.0f, 2.5f, 3.0f };
        default:
            return { 2.0f, 4.0f, 2.5f, 3.0f };
    }
}

// ── Optional room pool for filling remaining budget ─────────────────

static const InteriorRoomType OPTIONAL_POOL[] = {
    InteriorRoomType::PowerRelay,
    InteriorRoomType::Coolant,
    InteriorRoomType::LifeSupport,
    InteriorRoomType::Comms,
    InteriorRoomType::SensorCore,
    InteriorRoomType::Magazine,
    InteriorRoomType::Barracks,
    InteriorRoomType::Medbay,
    InteriorRoomType::MessHall,
    InteriorRoomType::MaintenanceShaft,
    InteriorRoomType::EscapePod,
};
static constexpr int OPTIONAL_POOL_SIZE = 11;

// ── Helper: make a room ─────────────────────────────────────────────

InteriorRoom InteriorGenerator::makeRoom(DeterministicRNG& rng,
                                          int roomId,
                                          InteriorRoomType type,
                                          const InteriorBudget& budget) {
    RoomSizeRange sr = sizeForType(type);
    InteriorRoom r{};
    r.roomId       = roomId;
    r.type         = type;
    r.dimX         = rng.rangeFloat(sr.minDim, sr.maxDim);
    r.dimY         = rng.rangeFloat(sr.minDim, sr.maxDim);
    r.dimZ         = rng.rangeFloat(sr.heightMin, sr.heightMax);
    r.pressurized  = true;
    r.deckLevel    = 0;  // assigned later

    // Enforce cockpit minimums.
    if (type == InteriorRoomType::Cockpit || type == InteriorRoomType::Bridge) {
        if (r.dimX < budget.minCockpitWidth) r.dimX = budget.minCockpitWidth;
        if (r.dimY < budget.minCockpitDepth) r.dimY = budget.minCockpitDepth;
        if (r.dimZ < budget.minCockpitHeight) r.dimZ = budget.minCockpitHeight;
    }

    // Power/heat/crew estimates.
    switch (type) {
        case InteriorRoomType::Reactor:
            r.powerDraw = -100.0f; // produces power
            r.heatOutput = 80.0f;
            r.crewCapacity = 4;
            break;
        case InteriorRoomType::Engineering:
            r.powerDraw = 20.0f;
            r.heatOutput = 30.0f;
            r.crewCapacity = 6;
            break;
        case InteriorRoomType::WeaponControl:
            r.powerDraw = 30.0f;
            r.heatOutput = 15.0f;
            r.crewCapacity = 3;
            break;
        case InteriorRoomType::ShieldCore:
            r.powerDraw = 40.0f;
            r.heatOutput = 25.0f;
            r.crewCapacity = 2;
            break;
        case InteriorRoomType::Cockpit:
        case InteriorRoomType::Bridge:
            r.powerDraw = 10.0f;
            r.heatOutput = 5.0f;
            r.crewCapacity = rng.range(2, 12);
            break;
        case InteriorRoomType::CrewQuarters:
        case InteriorRoomType::Barracks:
            r.powerDraw = 5.0f;
            r.heatOutput = 3.0f;
            r.crewCapacity = rng.range(4, 16);
            break;
        default:
            r.powerDraw = 5.0f;
            r.heatOutput = 2.0f;
            r.crewCapacity = rng.range(1, 4);
            break;
    }

    return r;
}

// ── Place mandatory rooms ───────────────────────────────────────────

void InteriorGenerator::placeMandatoryRooms(DeterministicRNG& rng,
                                             GeneratedInterior& interior) {
    MandatoryRoomSet mandatory = getMandatoryRooms(interior.shipClass);
    int nextId = 0;

    // Cockpit or Bridge depending on bridge type.
    InteriorRoomType cmdType = InteriorRoomType::Cockpit;
    if (interior.bridgeType == BridgeType::InlineArmored ||
        interior.bridgeType == BridgeType::RaisedTower ||
        interior.bridgeType == BridgeType::Hammerhead) {
        cmdType = InteriorRoomType::Bridge;
    }
    interior.rooms.push_back(makeRoom(rng, nextId++, cmdType, interior.budget));

    // Distributed CIC: add 3 command nodes instead of 1 bridge.
    if (interior.bridgeType == BridgeType::DistributedCIC) {
        int cicNodes = rng.range(3, 5);
        for (int i = 0; i < cicNodes; ++i) {
            interior.rooms.push_back(
                makeRoom(rng, nextId++, InteriorRoomType::CICNode, interior.budget));
        }
    }

    if (mandatory.engineering)
        interior.rooms.push_back(makeRoom(rng, nextId++, InteriorRoomType::Engineering, interior.budget));
    if (mandatory.reactor)
        interior.rooms.push_back(makeRoom(rng, nextId++, InteriorRoomType::Reactor, interior.budget));
    if (mandatory.weaponControl)
        interior.rooms.push_back(makeRoom(rng, nextId++, InteriorRoomType::WeaponControl, interior.budget));
    if (mandatory.crewQuarters)
        interior.rooms.push_back(makeRoom(rng, nextId++, InteriorRoomType::CrewQuarters, interior.budget));
    if (mandatory.cargoBay)
        interior.rooms.push_back(makeRoom(rng, nextId++, InteriorRoomType::CargoBay, interior.budget));
    if (mandatory.shieldCore)
        interior.rooms.push_back(makeRoom(rng, nextId++, InteriorRoomType::ShieldCore, interior.budget));
    if (mandatory.hangar)
        interior.rooms.push_back(makeRoom(rng, nextId++, InteriorRoomType::Hangar, interior.budget));
}

// ── Fill remaining budget ───────────────────────────────────────────

void InteriorGenerator::fillBudgetRooms(DeterministicRNG& rng,
                                         GeneratedInterior& interior) {
    int targetRooms = rng.range(interior.budget.roomCountMin,
                                interior.budget.roomCountMax);
    int currentCount = static_cast<int>(interior.rooms.size());
    int nextId = currentCount;

    while (currentCount < targetRooms) {
        InteriorRoomType type = OPTIONAL_POOL[rng.range(0, OPTIONAL_POOL_SIZE - 1)];
        interior.rooms.push_back(makeRoom(rng, nextId++, type, interior.budget));
        ++currentCount;
    }
}

// ── Distribute rooms across vertical decks ──────────────────────────

void InteriorGenerator::distributeAcrossDecks(DeterministicRNG& rng,
                                               GeneratedInterior& interior) {
    int deckCount = interior.budget.maxVerticalLevels;
    if (deckCount < 1) deckCount = 1;
    int roomCount = static_cast<int>(interior.rooms.size());
    if (deckCount > roomCount) deckCount = roomCount;
    interior.deckCount = deckCount;

    // Deck height (3m) and room spacing (6m) are consistent with the
    // deck_graph.h constants and the corridor length used below.
    static constexpr float DECK_HEIGHT = 3.0f;    // metres per deck
    static constexpr float ROOM_SPACING = 6.0f;   // metres between room centres

    // Assign deck levels round-robin, then position.
    for (int i = 0; i < roomCount; ++i) {
        InteriorRoom& r = interior.rooms[static_cast<size_t>(i)];
        r.deckLevel = i % deckCount;

        // Position along X per deck, stacked vertically.
        int roomsOnThisDeck = 0;
        for (int j = 0; j < i; ++j) {
            if (interior.rooms[static_cast<size_t>(j)].deckLevel == r.deckLevel)
                ++roomsOnThisDeck;
        }
        r.posX = static_cast<float>(roomsOnThisDeck) * ROOM_SPACING;
        r.posY = 0.0f;
        r.posZ = static_cast<float>(r.deckLevel) * DECK_HEIGHT;
    }

    // Command room always on top deck.
    if (!interior.rooms.empty()) {
        interior.rooms[0].deckLevel = deckCount - 1;
        interior.rooms[0].posZ = static_cast<float>(deckCount - 1) * DECK_HEIGHT;
    }
}

// ── Connect with corridors ──────────────────────────────────────────

void InteriorGenerator::connectWithCorridors(DeterministicRNG& rng,
                                              GeneratedInterior& interior) {
    float corridorWidth = rng.rangeFloat(interior.budget.corridorWidthMin,
                                          interior.budget.corridorWidthMax);
    int corridorId = 0;

    // Per-deck: connect consecutive rooms as main spine.
    for (int deck = 0; deck < interior.deckCount; ++deck) {
        std::vector<int> deckRoomIds;
        for (const auto& r : interior.rooms) {
            if (r.deckLevel == deck)
                deckRoomIds.push_back(r.roomId);
        }

        for (size_t i = 0; i + 1 < deckRoomIds.size(); ++i) {
            InteriorCorridor c{};
            c.corridorId = corridorId++;
            c.fromRoomId = deckRoomIds[i];
            c.toRoomId   = deckRoomIds[i + 1];
            c.width      = corridorWidth;
            c.length     = 6.0f; // matches ROOM_SPACING
            c.isMainSpine = true;

            // Register connections on rooms.
            interior.rooms[static_cast<size_t>(c.fromRoomId)].connections.push_back(c.toRoomId);
            interior.rooms[static_cast<size_t>(c.toRoomId)].connections.push_back(c.fromRoomId);

            interior.corridors.push_back(c);
        }
    }

    // Inter-deck vertical connections: connect room 0 on each deck pair.
    if (interior.deckCount > 1) {
        for (int deck = 0; deck + 1 < interior.deckCount; ++deck) {
            int fromId = -1, toId = -1;
            for (const auto& r : interior.rooms) {
                if (r.deckLevel == deck && fromId < 0) fromId = r.roomId;
                if (r.deckLevel == deck + 1 && toId < 0) toId = r.roomId;
            }
            if (fromId >= 0 && toId >= 0) {
                InteriorCorridor vc{};
                vc.corridorId = corridorId++;
                vc.fromRoomId = fromId;
                vc.toRoomId   = toId;
                vc.width      = corridorWidth * 0.8f;
                vc.length     = 3.0f; // one deck height
                vc.isMainSpine = false;

                interior.rooms[static_cast<size_t>(fromId)].connections.push_back(toId);
                interior.rooms[static_cast<size_t>(toId)].connections.push_back(fromId);
                interior.corridors.push_back(vc);
            }
        }
    }
}

// ── Add loop connections for survivability ───────────────────────────

void InteriorGenerator::addLoopConnections(DeterministicRNG& rng,
                                            GeneratedInterior& interior) {
    // Ships class 2+ (cruiser) get loop/flank routes.
    if (interior.shipClass < 2) return;

    int extraLoops = interior.shipClass - 1;
    int corridorId = static_cast<int>(interior.corridors.size());
    int roomCount  = static_cast<int>(interior.rooms.size());

    for (int i = 0; i < extraLoops && roomCount > 3; ++i) {
        int a = rng.range(0, roomCount - 1);
        int b = rng.range(0, roomCount - 1);
        if (a == b) continue;

        // Check they aren't already connected.
        bool alreadyConnected = false;
        for (int c : interior.rooms[static_cast<size_t>(a)].connections) {
            if (c == b) {
                alreadyConnected = true;
                break;
            }
        }
        if (alreadyConnected) continue;

        float dx = interior.rooms[static_cast<size_t>(a)].posX -
                   interior.rooms[static_cast<size_t>(b)].posX;
        float dy = interior.rooms[static_cast<size_t>(a)].posY -
                   interior.rooms[static_cast<size_t>(b)].posY;
        float dz = interior.rooms[static_cast<size_t>(a)].posZ -
                   interior.rooms[static_cast<size_t>(b)].posZ;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        InteriorCorridor lc{};
        lc.corridorId = corridorId++;
        lc.fromRoomId = interior.rooms[static_cast<size_t>(a)].roomId;
        lc.toRoomId   = interior.rooms[static_cast<size_t>(b)].roomId;
        lc.width      = interior.budget.corridorWidthMin;
        lc.length     = dist;
        lc.isMainSpine = false;

        interior.rooms[static_cast<size_t>(a)].connections.push_back(lc.toRoomId);
        interior.rooms[static_cast<size_t>(b)].connections.push_back(lc.fromRoomId);
        interior.corridors.push_back(lc);
    }
}

// ── Hull influences ─────────────────────────────────────────────────

void InteriorGenerator::computeHullInfluences(GeneratedInterior& interior) {
    interior.hullInfluences.reserve(interior.rooms.size());
    for (const auto& r : interior.rooms) {
        HullInfluence hi = computeHullInfluence(r.type, r.posX, r.posY, r.posZ);
        if (hi.magnitude > 0.0f) {
            interior.hullInfluences.push_back(hi);
        }
    }
}

// ── FPS validation pass ─────────────────────────────────────────────

bool InteriorGenerator::validateFPS(const GeneratedInterior& interior) {
    // Rule 1: every room except escape pods must have ≥ 2 connections
    //         (if room area > threshold).
    for (const auto& r : interior.rooms) {
        if (r.type == InteriorRoomType::EscapePod) continue;
        float area = r.dimX * r.dimY;
        if (area > 12.0f && r.connections.size() < 2) return false;
    }

    // Rule 2: must have at least one command room.
    bool hasCommand = false;
    for (const auto& r : interior.rooms) {
        if (r.type == InteriorRoomType::Cockpit ||
            r.type == InteriorRoomType::Bridge ||
            r.type == InteriorRoomType::CICNode) {
            hasCommand = true;
            break;
        }
    }
    if (!hasCommand) return false;

    // Rule 3: corridor count within budget.
    int corridorCount = static_cast<int>(interior.corridors.size());
    if (corridorCount < interior.budget.corridorCountMin) return false;

    return true;
}

// ── Public API ──────────────────────────────────────────────────────

GeneratedInterior InteriorGenerator::generate(const PCGContext& ctx,
                                               int shipClass) {
    if (shipClass < 0) shipClass = 0;
    if (shipClass > 5) shipClass = 5;

    DeterministicRNG rng(ctx.seed);
    GeneratedInterior interior{};
    interior.shipId    = ctx.seed;
    interior.shipClass = shipClass;
    interior.budget    = computeInteriorBudget(shipClass);
    interior.bridgeType = selectBridgeType(shipClass, rng);

    // Generation pipeline.
    placeMandatoryRooms(rng, interior);
    fillBudgetRooms(rng, interior);
    distributeAcrossDecks(rng, interior);
    connectWithCorridors(rng, interior);
    addLoopConnections(rng, interior);
    computeHullInfluences(interior);
    interior.valid = validateFPS(interior);

    return interior;
}

} // namespace pcg
} // namespace atlas
