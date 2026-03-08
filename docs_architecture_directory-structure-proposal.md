# NovaForge — Streamlined Directory Structure Proposal

Status: Draft (created 2026-03-08)

This document proposes a more streamlined, scalable directory structure for the `shifty81/NovaForge` repository, with special attention to adding large cross-cutting systems like **Chat** (client UI + networking + persistence + moderation).

> Note: This proposal is based on the **current top-level repo layout** only. A deeper, more precise plan should be finalized after reviewing the internal structure of `cpp_client/`, `cpp_server/`, `engine/`, and `editor/`.

---

## 1) Current top-level structure (observed)

```text
.github/
assets/
atlas_tests/
cpp_client/
cpp_server/
data/
docs/
editor/
engine/
schemas/
scripts/
specs/
testing/
tools/
+ root build/config files (CMakeLists.txt, Dockerfile, README.md, etc.)
```

This is already close to a good “monorepo” layout: engine + client + server + editor + assets + documentation.

---

## 2) Primary goals of streamlining

### 2.1 Reduce duplication between client/server
Cross-cutting systems like Chat need shared items:
- Protocol message definitions (message IDs, payload structs)
- Common ID types (player/character IDs)
- Error codes and rate-limit reasons
- Shared serialization helpers

If these live separately in `cpp_client/` and `cpp_server/`, they drift over time.

### 2.2 Keep layering clean
A consistent layering avoids spaghetti:
- **UI** should not directly talk to sockets
- **Networking** should not own UI logic
- **Services (domain)** should expose simple APIs (send message, subscribe to stream)
- **Server** should be authoritative (validation/routing/persistence)

### 2.3 Make room for big features (Chat, friends, guilds, inventory, etc.)
Large systems become maintainable when they have predictable “homes” in the tree.

---

## 3) Recommended approach: “Minimal disruption + shared common library”

Rather than renaming large folders immediately, the most practical streamlining step is:

### Add a shared `common/` (or `cpp_common/`) library
This becomes the single source of truth for:
- Protocol messages + serialization rules
- Shared types (IDs, timestamps, enums)
- Shared utilities that are not UI-specific and not server-only

Example:

```text
common/
  include/
    novaforge/
      common/
        ids.h
        time.h
        error_codes.h
      protocol/
        message_id.h
        chat_messages.h
        admin_console_messages.h
        serialization.h
  src/ (optional; only if you need compiled translation units)
```

Both `cpp_client` and `cpp_server` link against `common`.

---

## 4) Target “streamlined” repo structure (recommended)

This is a target layout to evolve toward over time. You can adopt it gradually.

```text
engine/                 # engine core libs (render, ECS, math, platform)
common/                 # shared types + protocol + serialization helpers
cpp_client/             # game client app
cpp_server/             # game server app/services
editor/                 # tools/editor runtime

assets/
data/

docs/
specs/
schemas/
scripts/
tools/
tests/                  # recommended: unify atlas_tests + testing into tests/
cmake/                  # optional: CMake modules/toolchains/helpers
```

> Optional future rename:
> - `cpp_client/` → `client/`
> - `cpp_server/` → `server/`
> Do this only if/when you want a larger cleanup.

---

## 5) Recommended internal layout (client/server)

This is the structure that keeps Chat (and similar systems) clean.

### 5.1 Client internals (conceptual)
```text
cpp_client/
  include/
    novaforge/client/...
  src/
    app/                 # application bootstrap, main loop, state management
    ui/
      hud/
      widgets/
        chat/            # chat UI widgets
    net/
      transport/         # TCP/UDP sockets, connection, reliability primitives
      protocol_handlers/ # mapping protocol messages -> service calls
    services/
      chat/              # ChatService, ChatStore, history, ack logic
```

### 5.2 Server internals (conceptual)
```text
cpp_server/
  include/
    novaforge/server/...
  src/
    net/
      transport/         # TCP/UDP sockets, session management
      protocol_handlers/ # decode -> validate -> dispatch
    services/
      chat/              # authoritative routing, validation, rate limits
    persistence/
      chat/              # storage + retention (90 days), pruning jobs
```

---

## 6) Chat-specific placement (matches “no ImGui, custom UI”)

Chat is naturally split into three layers:

### 6.1 UI layer (client)
- `cpp_client/src/ui/widgets/chat/`
- Components:
  - `ChatWidget` (history view + input + tab bar)
  - `ChatTabBar` (tabs + flashing/unread badges)
  - `ChatDMTabView` (DM sub-tabs)
  - `ChatSystemTabView` (System category sub-tabs)

### 6.2 Domain/service layer (client)
- `cpp_client/src/services/chat/`
- Components:
  - `ChatService` (API used by UI: Send, OpenDM, RequestHistory)
  - `ChatStore` (ring buffers; last 1000; unread/flash tracking)
  - `ChatStreamKey` (Global/Local(solarSystemId)/Party/Guild/Whisper(threadId)/System)
  - Delivery semantics:
    - No out-of-order display
    - Guaranteed delivery using ack/retry or reliable ordered transport

### 6.3 Protocol layer (shared)
- `common/include/novaforge/protocol/chat_messages.h`
- Defines message payloads:
  - `ChatSend`
  - `ChatMessage`
  - `ChatAck`
  - `ChatSendResult`
  - `ChatHistoryChunk`
  - `AdminCommandSend` / `AdminCommandResult` (Server tab / console)

---

## 7) Tests + docs consolidation (recommended)
You currently have both:
- `atlas_tests/`
- `testing/`

Recommendation: consolidate to:

```text
tests/
  unit/
  integration/
  load/
```

If there is a strong reason to keep both, define clear intent:
- `atlas_tests/` = engine-level tests for Atlas components
- `testing/` = gameplay/server integration tests

But unifying under `tests/` reduces confusion.

---

## 8) Migration strategy (practical step-by-step)

### Phase A: Add common/ without moving existing code
1. Create `common/` CMake target (or equivalent).
2. Move (or duplicate initially) protocol definitions into `common/`.
3. Update client/server builds to link `common/`.

### Phase B: Add Chat with minimal disruption
1. Add `common/protocol/chat_messages.*`.
2. Add server chat service + persistence hooks.
3. Add client ChatService + ChatStore.
4. Add custom UI widgets under client UI tree.
5. Wire input focus (Enter to open chat, Esc to close) into your existing input system.

### Phase C: Optional cleanup
1. Consolidate `testing/` and `atlas_tests/` into `tests/` if desired.
2. Consider renaming `cpp_client/` and `cpp_server/` for brevity.
3. Add `cmake/` folder if build helpers grow.

---

## 9) Notes / open items for finalizing this plan
To finalize a “deep dive” version of this proposal, we should inspect:
- Where the current custom UI widgets live
- Where key/text input is handled
- Where networking messages are encoded/decoded and dispatched
- Whether there is already a shared types/protocol library pattern

Once those are identified, we can produce:
- An exact file-by-file placement for Chat
- A concrete set of refactors that improve cohesion without breaking builds

---