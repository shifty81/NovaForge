# NovaForge — Streamlined Directory Structure Proposal

**Status:** Draft
**Date:** 2026-03-08

This document proposes a more streamlined, scalable directory structure for `shifty81/NovaForge`, with special attention to cross-cutting systems like **Chat** (UI + networking + persistence + moderation).

---

## Current Structure

```
NovaForge/
├── .github/           # CI workflows
├── assets/            # Game assets (textures, models, sounds)
├── atlas_tests/       # Atlas engine / editor tests (headless CI)
├── cpp_client/        # Game client (C++/OpenGL)
│   ├── include/       # Headers
│   │   ├── core/      # Application, camera, session
│   │   ├── network/   # TCP client, protocol, network manager
│   │   ├── rendering/ # OpenGL renderer, shaders, materials
│   │   └── ui/        # RML UI, Atlas widgets, HUD
│   └── src/           # Implementations
├── cpp_server/        # Authoritative game server
│   ├── include/       # Headers
│   │   ├── components/# ECS component definitions (15 files, 11K+ lines)
│   │   ├── ecs/       # Entity, World, System base classes
│   │   ├── handlers/  # Message handlers (Combat, Station, Movement, etc.)
│   │   ├── network/   # TCP server, protocol handler
│   │   └── systems/   # 316 ECS systems
│   ├── src/           # Implementations
│   └── tests/         # Per-system test suites (363 files, 12K+ assertions)
├── data/              # JSON config, ship definitions, missions
├── docs/              # Documentation (ROADMAP, guides, design docs)
├── editor/            # Atlas Editor (tools, panels, commands)
└── engine/            # Atlas Engine core (ECS, math, audio, net, animation)
```

---

## Identified Issues

1. **No shared protocol library** — Client and server each define `ProtocolHandler` independently. Message types are duplicated.
2. **No shared domain types** — Player IDs, character IDs, channel enums defined separately in client and server.
3. **Cross-cutting features** (like Chat) require touching both `cpp_client/` and `cpp_server/` with no shared contract.
4. **Test organization** — `atlas_tests/` (engine/editor) and `cpp_server/tests/` (ECS systems) use different frameworks.

---

## Proposed Changes

### Phase 1: Add `cpp_common/` Shared Library (Minimal Disruption)

Create a new shared library that both client and server link against:

```
cpp_common/
├── CMakeLists.txt
└── include/
    └── novaforge/
        ├── protocol/
        │   ├── message_types.h      # Shared MessageType enum
        │   ├── chat_messages.h      # Chat-specific message structs
        │   └── serialization.h      # Shared JSON helpers
        ├── types/
        │   ├── player_id.h          # Strong-typed player/character IDs
        │   ├── channel_types.h      # ChatStreamType, SystemCategory enums
        │   └── time_types.h         # Timestamp utilities
        └── chat/
            ├── chat_types.h         # ChatMessage, ChatSendResult structs
            └── chat_constants.h     # Rate limits, max lengths, retention
```

**Impact:** No existing files need to move. Client and server add `cpp_common` as a dependency.

### Phase 2: Client Chat Integration

Add chat-specific code under the existing client structure:

```
cpp_client/
├── include/
│   ├── chat/
│   │   ├── chat_service.h           # Domain API (send/receive, ring buffer)
│   │   └── chat_store.h             # Per-stream message storage
│   └── ui/
│       └── atlas/
│           ├── chat_widget.h        # Chat panel (tabs, input, scrollback)
│           └── chat_tab_bar.h       # Tab bar with flash/unread indicators
└── src/
    ├── chat/
    │   ├── chat_service.cpp
    │   └── chat_store.cpp
    └── ui/
        └── atlas/
            ├── chat_widget.cpp
            └── chat_tab_bar.cpp
```

### Phase 3: Server Chat Handler

Add dedicated chat handler following existing `IMessageHandler` pattern:

```
cpp_server/
├── include/
│   └── handlers/
│       └── chat_handler.h           # IMessageHandler for chat messages
└── src/
    └── handlers/
        └── chat_handler.cpp
```

---

## Future Phases (Not in This PR)

### Phase 4: Consolidate Protocol Definitions

Migrate `MessageType` enum from both `cpp_server/include/network/protocol_handler.h` and `cpp_client/include/network/protocol_handler.h` into `cpp_common/include/novaforge/protocol/message_types.h`.

### Phase 5: Rename Directories (Optional, Breaking)

| Current | Proposed | Rationale |
|---------|----------|-----------|
| `cpp_client/` | `client/` | Shorter, C++ is implied |
| `cpp_server/` | `server/` | Shorter, C++ is implied |
| `atlas_tests/` | `tests/atlas/` | Group all tests |

**Note:** This is a large breaking change that should be done in a dedicated PR with full CI verification.

---

## Build Integration

### CMake Changes

```cmake
# cpp_common/CMakeLists.txt
add_library(novaforge_common INTERFACE)
target_include_directories(novaforge_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# cpp_server/CMakeLists.txt — add dependency
target_link_libraries(novaforge_core PUBLIC novaforge_common)

# cpp_client/CMakeLists.txt — add dependency
target_link_libraries(atlas_client PUBLIC novaforge_common)
```

---

## Summary

| Phase | Scope | Breaking? | Files Moved? |
|-------|-------|-----------|-------------|
| 1 | Add `cpp_common/` | No | No |
| 2 | Client chat code | No | No |
| 3 | Server chat handler | No | No |
| 4 | Consolidate protocol | Low risk | Yes (copy + alias) |
| 5 | Rename dirs | High risk | Yes |

**Recommendation:** Implement Phases 1–3 now. Phases 4–5 can be done incrementally in future PRs.
