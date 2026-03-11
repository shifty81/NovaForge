# Copilot Instructions for NovaForge

NovaForge is a PVE space simulator built with C++17. The repository contains
four main sub-projects:

| Directory | Description |
|-----------|-------------|
| `cpp_server/` | Dedicated game server — ECS, systems, components, tests |
| `cpp_client/` | OpenGL game client |
| `engine/` | Atlas Engine — ECS core, rendering, audio, PCG |
| `editor/` | Atlas Editor GUI tool |

---

## Coding conventions

The full guidelines live in [`docs/CODING_GUIDELINES.md`](../docs/CODING_GUIDELINES.md).
Key rules Copilot must follow:

### File naming
- `cpp_server/` and `cpp_client/`: **snake_case** (`combat_system.h`, `ship_physics.cpp`)
- `engine/` and `editor/`: **PascalCase** (`EventBus.h`, `ECS.cpp`)

### Naming inside C++
- Classes: `PascalCase` everywhere
- Methods: `snake_case` in `cpp_server/` and `cpp_client/`; `PascalCase` in `engine/` and `editor/`
- Member variables: trailing `snake_case_` in `cpp_server/`; `m_camelCase` in `engine/`
- Local variables and parameters: `snake_case` in `cpp_server/`

### Namespaces
All code lives under `namespace atlas`. Sub-namespaces: `ecs`, `systems`,
`components`, `network`, `pcg`, `sim`. Always use the two-level form:
```cpp
namespace atlas {
namespace systems {
// ...
} // namespace systems
} // namespace atlas
```

### Headers
- Use `#ifndef` include guards matching the pattern `NOVAFORGE_<SUBSYSTEM>_<FILE>_H`
  (existing files use this style; do not switch to `#pragma once` in `cpp_server/`).
- Include ordering in `.cpp` files: own header → project headers → third-party → stdlib.

### No raw `cout`/`cerr` in `cpp_server/`
Use `atlas::Logger` instead.

---

## ECS system pattern (`cpp_server/`)

Every game-logic system follows this pattern.

### Component (in `cpp_server/include/components/<category>_components.h`)
```cpp
class MyState : public ecs::Component {
public:
    // Data fields (no trailing underscore — components are plain data)
    int count = 0;
    bool active = true;
    COMPONENT_TYPE(MyState)
};
```

### System header (`cpp_server/include/systems/my_system.h`)
```cpp
#ifndef NOVAFORGE_SYSTEMS_MY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/<category>_components.h"
#include <string>

namespace atlas {
namespace systems {

class MySystem : public ecs::SingleComponentSystem<components::MyState> {
public:
    explicit MySystem(ecs::World* world);
    ~MySystem() override = default;

    std::string getName() const override { return "MySystem"; }

    bool initialize(const std::string& entity_id);
    // ... other public methods

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MyState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MY_SYSTEM_H
```

### System implementation (`cpp_server/src/systems/my_system.cpp`)
```cpp
#include "systems/my_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

MySystem::MySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MySystem::updateComponent(ecs::Entity& /*entity*/,
                                components::MyState& comp,
                                float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool MySystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MyState>();
    entity->addComponent(std::move(comp));
    return true;
}

} // namespace systems
} // namespace atlas
```

Key rules:
- **All mutating methods return `bool`**: `true` on success, `false` when the
  entity is missing or the operation is invalid.
- **All query methods return a safe default** (0, 0.0f, false, empty string)
  when the entity/component is absent.
- `getComponentFor(entity_id)` is the standard way to retrieve the component.
- The `updateComponent` tick is driven by the ECS world — per-frame logic goes
  there; everything else is driven by explicit method calls.

---

## Adding a new ECS system

1. **Add the component** to the appropriate `cpp_server/include/components/<category>_components.h`.
2. **Create the header** `cpp_server/include/systems/<name>_system.h`.
3. **Create the implementation** `cpp_server/src/systems/<name>_system.cpp`.
4. **Create the test file** `cpp_server/tests/test_<name>_system.cpp`
   (see test conventions below).
5. **Register in `cpp_server/CMakeLists.txt`**:
   - Add `src/systems/<name>_system.cpp` to `CORE_SOURCES`.
   - Add `include/systems/<name>_system.h` to the header list.
6. **Register in `cpp_server/tests/test_main.cpp`**:
   - Add `void run_<name>_system_tests();` forward declaration.
   - Call `run_<name>_system_tests();` in `main()`.

---

## Test conventions (`cpp_server/tests/`)

Tests use a custom lightweight framework. Every test file has this shape:

```cpp
// Tests for: MySystem
#include "test_log.h"
#include "components/<category>_components.h"
#include "ecs/system.h"
#include "systems/my_system.h"

using namespace atlas;

static void testMySystemInit() {
    std::cout << "\n=== MySystem: Init ===" << std::endl;
    ecs::World world;
    systems::MySystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    // ...
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

// ... more test functions

void run_my_system_tests() {
    testMySystemInit();
    // call all test functions
}
```

Rules:
- Every system needs a **"Missing" test** — verify that every method returns
  a safe value / `false` when the entity does not exist.
- Use `assertTrue(expr, "message")` and `approxEqual(a, b)` from `test_log.h`.
- Do **not** call `sys.update(*entity, delta)` — use `sys.update(delta)` (the
  single-argument form from `SingleComponentSystem`).
- Target ≥ 50 assertions per system.

---

## Build commands

### Server only (fastest for iteration)

```bash
cd cpp_server
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DUSE_STEAM_SDK=OFF
cmake --build . -j$(nproc)
```

### Build + run a single system's tests (preferred during development)

```bash
cd cpp_server/build
cmake --build . --target test_<name>_system -j$(nproc)
./bin/test_<name>_system
```

### Run all server tests

```bash
cd cpp_server
./run_tests.sh
# Or manually:
cd cpp_server/build
./bin/test_systems
```

### Full project build (engine + editor + client + server)

```bash
./scripts/build_all.sh --test
```

---

## Repository layout reference

```
cpp_server/
  include/
    components/          ← Component definitions (one file per domain)
    systems/             ← System headers
    ecs/                 ← World, Entity, SingleComponentSystem base
    ui/                  ← Server console UI
    utils/               ← Shared utilities
  src/
    systems/             ← System implementations (mirrors include/systems/)
    ...
  tests/
    test_main.cpp        ← Monolithic test runner (all suites)
    test_single_main.cpp ← Single-system runner template
    test_log.h           ← assertTrue / approxEqual helpers
    test_<name>_system.cpp ← Per-system test files
  CMakeLists.txt         ← Add every new .cpp and .h here

docs/
  BUILDING.md            ← Full build instructions
  CODING_GUIDELINES.md   ← Full style rules
  CONTRIBUTING.md        ← Contribution workflow
  archive/NEXT_TASKS.md  ← Running log of implemented systems and assertion counts
  design/EVE_FEATURE_GAP.md ← Checklist of EVE mechanics implemented
```

---

## Current project state

- **18,747+ server test assertions** passing across 363+ ECS systems.
- Systems are grouped in `cpp_server/include/components/` by domain:
  `combat_components.h`, `game_components.h`, `economy_components.h`,
  `navigation_components.h`, `exploration_components.h`, etc.
- When continuing work, check `docs/archive/NEXT_TASKS.md` for the latest
  progress update and any systems still missing implementations.
- To find systems that have a header but no `.cpp` / test yet, run:

```bash
cd cpp_server
for h in include/systems/*.h; do
  base=$(basename $h .h)
  [ ! -f "src/systems/${base}.cpp" ] || [ ! -f "tests/test_${base}.cpp" ] && echo "MISSING: $base"
done
```
