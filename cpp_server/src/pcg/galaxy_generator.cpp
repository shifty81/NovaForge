#include "pcg/galaxy_generator.h"
#include "pcg/hash_utils.h"
#include "pcg/deterministic_rng.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace atlas {
namespace pcg {

// ── Constants ──────────────────────────────────────────────────────
static constexpr int   MIN_SYSTEMS        = 10;
static constexpr int   MAX_SYSTEMS        = 500;
static constexpr float TWO_PI             = 6.2831853f;
static constexpr float DISC_RADIUS        = 500.0f;   // light-years
static constexpr float DISC_HEIGHT        = 40.0f;    // light-years (thin disc)
static constexpr float CORE_RADIUS_FRAC   = 0.30f;    // inner 30% = high-sec
static constexpr float MID_RADIUS_FRAC    = 0.65f;    // 30%-65%  = low-sec
                                                       // 65%-100% = null-sec
static constexpr int   MIN_PER_CONST      = 4;
static constexpr int   MAX_PER_CONST      = 8;
static constexpr int   MIN_CONST_PER_REG  = 3;
static constexpr int   MAX_CONST_PER_REG  = 5;
static constexpr int   MAX_NEIGHBORS      = 6;
static constexpr float BASE_CONN_DIST     = 60.0f;    // base connection threshold (ly)

// ── Helpers ────────────────────────────────────────────────────────

static float distance3d(float x1, float y1, float z1,
                        float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

static float radialDist(float x, float z) {
    return std::sqrt(x * x + z * z);
}

// ── Public API ─────────────────────────────────────────────────────

GeneratedGalaxy GalaxyGenerator::generate(const PCGContext& ctx,
                                          int systemCount) {
    // Clamp system count.
    if (systemCount < MIN_SYSTEMS) systemCount = MIN_SYSTEMS;
    if (systemCount > MAX_SYSTEMS) systemCount = MAX_SYSTEMS;

    DeterministicRNG rng(ctx.seed);

    GeneratedGalaxy galaxy{};
    galaxy.seed          = ctx.seed;
    galaxy.total_systems = systemCount;

    generatePositions(rng, galaxy, systemCount, ctx.seed);
    buildConnections(rng, galaxy);
    assignSecurity(galaxy);
    identifyChokepoints(galaxy);

    // Tallies.
    galaxy.highsec_count = 0;
    galaxy.lowsec_count  = 0;
    galaxy.nullsec_count = 0;
    for (const auto& node : galaxy.nodes) {
        switch (node.security) {
            case SecurityZone::HighSec: ++galaxy.highsec_count; break;
            case SecurityZone::LowSec:  ++galaxy.lowsec_count;  break;
            case SecurityZone::NullSec: ++galaxy.nullsec_count; break;
        }
    }

    galaxy.valid = !galaxy.nodes.empty() && !galaxy.routes.empty();
    return galaxy;
}

std::string GalaxyGenerator::securityZoneName(SecurityZone zone) {
    switch (zone) {
        case SecurityZone::HighSec: return "High-Sec";
        case SecurityZone::LowSec:  return "Low-Sec";
        case SecurityZone::NullSec: return "Null-Sec";
    }
    return "Unknown";
}

// ── Position generation ────────────────────────────────────────────
//
// Systems are distributed in a disc.  The inner core is denser (more
// high-sec systems), produced by biasing the radial distribution
// toward the centre for a fraction of the points.

void GalaxyGenerator::generatePositions(DeterministicRNG& rng,
                                        GeneratedGalaxy& galaxy,
                                        int systemCount,
                                        uint64_t baseSeed) {
    galaxy.nodes.reserve(static_cast<size_t>(systemCount));

    // Determine constellation / region grouping sizes.
    int constelSize = rng.range(MIN_PER_CONST, MAX_PER_CONST);
    int regSize     = rng.range(MIN_CONST_PER_REG, MAX_CONST_PER_REG) * constelSize;

    for (int i = 0; i < systemCount; ++i) {
        GalaxyNode node{};
        node.system_id = deriveSeed(baseSeed, static_cast<uint64_t>(i));

        // Radial distance — bias the first 30% of systems toward the core.
        float r;
        if (i < systemCount * 3 / 10) {
            // Core cluster: radius within inner 35% of disc.
            r = rng.rangeFloat(0.0f, DISC_RADIUS * 0.35f);
        } else if (i < systemCount * 65 / 100) {
            // Mid ring.
            r = rng.rangeFloat(DISC_RADIUS * 0.20f, DISC_RADIUS * 0.70f);
        } else {
            // Outer ring.
            r = rng.rangeFloat(DISC_RADIUS * 0.50f, DISC_RADIUS);
        }

        float angle = rng.rangeFloat(0.0f, TWO_PI);
        node.x = r * std::cos(angle);
        node.z = r * std::sin(angle);
        node.y = rng.rangeFloat(-DISC_HEIGHT * 0.5f, DISC_HEIGHT * 0.5f);

        // Constellation & region assignment (sequential grouping).
        node.constellation_id = static_cast<uint32_t>(i / constelSize);
        node.region_id        = static_cast<uint32_t>(i / regSize);

        galaxy.nodes.push_back(node);
    }
}

// ── Connection building ────────────────────────────────────────────
//
// For each system, connect to the closest neighbours within a distance
// threshold.  Then ensure every node has at least one connection so the
// graph has no isolated vertices.

void GalaxyGenerator::buildConnections(DeterministicRNG& rng,
                                       GeneratedGalaxy& galaxy) {
    int n = static_cast<int>(galaxy.nodes.size());
    if (n < 2) return;

    // Adaptive connection distance — scales with system density.
    float scaledDist = BASE_CONN_DIST * std::sqrt(static_cast<float>(MAX_SYSTEMS)
                                                   / static_cast<float>(n));
    if (scaledDist < BASE_CONN_DIST) scaledDist = BASE_CONN_DIST;

    // Track which pairs are already connected.
    // Use a flat vector of sets of neighbour indices for O(1) lookup.
    std::vector<std::vector<int>> adjList(static_cast<size_t>(n));

    auto addEdge = [&](int a, int b, float dist) {
        // Avoid duplicates.
        for (int nb : adjList[static_cast<size_t>(a)]) {
            if (nb == b) return;
        }
        adjList[static_cast<size_t>(a)].push_back(b);
        adjList[static_cast<size_t>(b)].push_back(a);

        GalaxyRoute route{};
        route.from_id      = galaxy.nodes[static_cast<size_t>(a)].system_id;
        route.to_id        = galaxy.nodes[static_cast<size_t>(b)].system_id;
        route.distance     = dist;
        route.is_chokepoint = false;
        galaxy.routes.push_back(route);
    };

    // Pass 1: connect each system to its closest neighbours within threshold.
    for (int i = 0; i < n; ++i) {
        const auto& ni = galaxy.nodes[static_cast<size_t>(i)];

        // Collect candidate neighbours sorted by distance.
        struct Candidate { int idx; float dist; };
        std::vector<Candidate> candidates;
        candidates.reserve(static_cast<size_t>(n));

        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            const auto& nj = galaxy.nodes[static_cast<size_t>(j)];
            float d = distance3d(ni.x, ni.y, ni.z, nj.x, nj.y, nj.z);
            if (d <= scaledDist) {
                candidates.push_back({j, d});
            }
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](const Candidate& a, const Candidate& b) {
                      return a.dist < b.dist;
                  });

        // Connect to up to MAX_NEIGHBORS closest.
        int added = 0;
        for (const auto& c : candidates) {
            if (added >= MAX_NEIGHBORS) break;
            if (static_cast<int>(adjList[static_cast<size_t>(i)].size()) >= MAX_NEIGHBORS) break;
            addEdge(i, c.idx, c.dist);
            ++added;
        }
    }

    // Pass 2: ensure no isolated nodes — connect each unconnected node to
    // its nearest neighbour.
    for (int i = 0; i < n; ++i) {
        if (!adjList[static_cast<size_t>(i)].empty()) continue;

        float bestDist = (std::numeric_limits<float>::max)();
        int bestIdx    = -1;
        const auto& ni = galaxy.nodes[static_cast<size_t>(i)];
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            const auto& nj = galaxy.nodes[static_cast<size_t>(j)];
            float d = distance3d(ni.x, ni.y, ni.z, nj.x, nj.y, nj.z);
            if (d < bestDist) {
                bestDist = d;
                bestIdx  = j;
            }
        }
        if (bestIdx >= 0) {
            addEdge(i, bestIdx, bestDist);
        }
    }

    // Pass 3: ensure graph connectivity via BFS.  If multiple components
    // exist, bridge the closest pair of nodes across components.
    std::vector<int> component(static_cast<size_t>(n), -1);
    int numComponents = 0;
    for (int i = 0; i < n; ++i) {
        if (component[static_cast<size_t>(i)] >= 0) continue;
        int compId = numComponents++;
        std::vector<int> stack;
        stack.push_back(i);
        component[static_cast<size_t>(i)] = compId;
        while (!stack.empty()) {
            int cur = stack.back();
            stack.pop_back();
            for (int nb : adjList[static_cast<size_t>(cur)]) {
                if (component[static_cast<size_t>(nb)] < 0) {
                    component[static_cast<size_t>(nb)] = compId;
                    stack.push_back(nb);
                }
            }
        }
    }

    // Bridge disconnected components.
    while (numComponents > 1) {
        float bestDist = (std::numeric_limits<float>::max)();
        int bestA = -1, bestB = -1;
        for (int i = 0; i < n; ++i) {
            if (component[static_cast<size_t>(i)] != 0) continue;
            const auto& ni = galaxy.nodes[static_cast<size_t>(i)];
            for (int j = 0; j < n; ++j) {
                if (component[static_cast<size_t>(j)] == 0) continue;
                const auto& nj = galaxy.nodes[static_cast<size_t>(j)];
                float d = distance3d(ni.x, ni.y, ni.z, nj.x, nj.y, nj.z);
                if (d < bestDist) {
                    bestDist = d;
                    bestA    = i;
                    bestB    = j;
                }
            }
        }
        if (bestA < 0 || bestB < 0) break;

        addEdge(bestA, bestB, bestDist);

        // Merge component of bestB into component 0.
        int mergedComp = component[static_cast<size_t>(bestB)];
        for (int k = 0; k < n; ++k) {
            if (component[static_cast<size_t>(k)] == mergedComp) {
                component[static_cast<size_t>(k)] = 0;
            }
        }
        --numComponents;
    }

    // Populate neighbour lists on nodes.
    for (int i = 0; i < n; ++i) {
        auto& node = galaxy.nodes[static_cast<size_t>(i)];
        node.neighbor_ids.clear();
        node.neighbor_ids.reserve(adjList[static_cast<size_t>(i)].size());
        for (int nb : adjList[static_cast<size_t>(i)]) {
            node.neighbor_ids.push_back(
                galaxy.nodes[static_cast<size_t>(nb)].system_id);
        }
    }
}

// ── Security assignment ────────────────────────────────────────────
//
// Based on radial distance from galactic centre:
//   Core  (0 – 30% radius):  HighSec, security_level 0.5 – 1.0
//   Mid   (30 – 65% radius): LowSec,  security_level 0.1 – 0.5
//   Outer (65 – 100% radius): NullSec, security_level 0.0 – 0.1

void GalaxyGenerator::assignSecurity(GeneratedGalaxy& galaxy) {
    // Find the maximum radial distance for normalisation.
    float maxR = 1.0f;
    for (const auto& node : galaxy.nodes) {
        float r = radialDist(node.x, node.z);
        if (r > maxR) maxR = r;
    }

    for (auto& node : galaxy.nodes) {
        float r    = radialDist(node.x, node.z);
        float frac = r / maxR;   // 0.0 = centre, 1.0 = edge

        if (frac <= CORE_RADIUS_FRAC) {
            node.security       = SecurityZone::HighSec;
            // Map [0, 0.30] → [1.0, 0.5]
            float t = frac / CORE_RADIUS_FRAC;   // 0..1
            node.security_level = 1.0f - t * 0.5f;
        } else if (frac <= MID_RADIUS_FRAC) {
            node.security       = SecurityZone::LowSec;
            // Map [0.30, 0.65] → [0.5, 0.1]
            float t = (frac - CORE_RADIUS_FRAC)
                    / (MID_RADIUS_FRAC - CORE_RADIUS_FRAC);
            node.security_level = 0.5f - t * 0.4f;
        } else {
            node.security       = SecurityZone::NullSec;
            // Map [0.65, 1.0] → [0.1, 0.0]
            float t = (frac - MID_RADIUS_FRAC)
                    / (1.0f - MID_RADIUS_FRAC);
            node.security_level = 0.1f - t * 0.1f;
            if (node.security_level < 0.0f) node.security_level = 0.0f;
        }
    }
}

// ── Chokepoint identification ──────────────────────────────────────
//
// A route is a chokepoint (bridge edge) if removing it increases the
// number of connected components.  We detect bridges using a DFS-based
// algorithm (Tarjan's bridge-finding).

void GalaxyGenerator::identifyChokepoints(GeneratedGalaxy& galaxy) {
    int n = static_cast<int>(galaxy.nodes.size());
    if (n < 2) return;

    // Build adjacency list indexed by node position.
    // Also build a map from (i,j) to route index for marking.
    struct Edge { int to; int routeIdx; };
    std::vector<std::vector<Edge>> adj(static_cast<size_t>(n));

    // Map system_id → node index.
    std::vector<uint64_t> idByIdx(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        idByIdx[static_cast<size_t>(i)] = galaxy.nodes[static_cast<size_t>(i)].system_id;
    }

    auto findIdx = [&](uint64_t sysId) -> int {
        for (int i = 0; i < n; ++i) {
            if (idByIdx[static_cast<size_t>(i)] == sysId) return i;
        }
        return -1;
    };

    int routeCount = static_cast<int>(galaxy.routes.size());
    for (int ri = 0; ri < routeCount; ++ri) {
        int a = findIdx(galaxy.routes[static_cast<size_t>(ri)].from_id);
        int b = findIdx(galaxy.routes[static_cast<size_t>(ri)].to_id);
        if (a < 0 || b < 0) continue;
        adj[static_cast<size_t>(a)].push_back({b, ri});
        adj[static_cast<size_t>(b)].push_back({a, ri});
    }

    // Tarjan's bridge-finding algorithm.
    std::vector<int> disc(static_cast<size_t>(n), -1);
    std::vector<int> low(static_cast<size_t>(n), -1);
    int timer = 0;

    // Iterative DFS to avoid stack overflow on large graphs.
    struct Frame {
        int node;
        int parent;
        int edgePos;   // index into adj[node]
    };

    for (int start = 0; start < n; ++start) {
        if (disc[static_cast<size_t>(start)] >= 0) continue;

        std::vector<Frame> stack;
        stack.push_back({start, -1, 0});
        disc[static_cast<size_t>(start)] = low[static_cast<size_t>(start)] = timer++;

        while (!stack.empty()) {
            Frame& f = stack.back();
            int u = f.node;
            auto& edges = adj[static_cast<size_t>(u)];

            if (f.edgePos < static_cast<int>(edges.size())) {
                int v  = edges[static_cast<size_t>(f.edgePos)].to;
                int ri = edges[static_cast<size_t>(f.edgePos)].routeIdx;
                ++f.edgePos;

                if (disc[static_cast<size_t>(v)] < 0) {
                    disc[static_cast<size_t>(v)] = low[static_cast<size_t>(v)] = timer++;
                    stack.push_back({v, u, 0});
                } else if (v != f.parent) {
                    if (low[static_cast<size_t>(u)] > disc[static_cast<size_t>(v)]) {
                        low[static_cast<size_t>(u)] = disc[static_cast<size_t>(v)];
                    }
                }
                (void)ri;  // used during back-propagation below
            } else {
                stack.pop_back();
                if (!stack.empty()) {
                    Frame& pf = stack.back();
                    int pu = pf.node;
                    // Back-propagate low value.
                    if (low[static_cast<size_t>(pu)] > low[static_cast<size_t>(u)]) {
                        low[static_cast<size_t>(pu)] = low[static_cast<size_t>(u)];
                    }
                    // Check bridge condition: low[u] > disc[pu] means the
                    // edge (pu, u) is a bridge.
                    if (low[static_cast<size_t>(u)] > disc[static_cast<size_t>(pu)]) {
                        // Find route index for this edge and mark as chokepoint.
                        for (const auto& e : adj[static_cast<size_t>(pu)]) {
                            if (e.to == u) {
                                galaxy.routes[static_cast<size_t>(e.routeIdx)].is_chokepoint = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

} // namespace pcg
} // namespace atlas
