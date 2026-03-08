# NovaForge — Chat System (Full Design Spec)

**Status:** Draft v1
**Date:** 2026-03-08
**Scope:** Implement full in-game Chat in `shifty81/NovaForge` with **custom UI** (no ImGui), server-authoritative enforcement, guaranteed delivery, strict in-order display, emojis, tabs w/ activity flashing, and persistence (90-day retention).

---

## 1. Tab / Channel Model

### Top-Level Tabs

| Tab | Channel Type | Scope | Notes |
|-----|-------------|-------|-------|
| **Global** | `global` | All players on shard | Broadcast |
| **Local** | `local` | Same solar system | Server determines recipients by `solar_system_id` |
| **Party** | `party` | Party members | Keyed by `party_id` |
| **Guild** | `guild` | Guild/Corp members | Keyed by `guild_id` |
| **Whisper** | `whisper` | 1:1 DMs | Container tab with **DM sub-tabs per peer** |
| **System** | `system` | Server announcements, moderation, errors | Sub-tabs: All, Moderation, Errors, Announcements |
| **Server** | `admin_console` | Admin-only console | Command input + server output stream |

### Sub-Tabs

- **Whisper** → DM sub-tabs per peer character (e.g., "DM: Luna")
- **System** → Category sub-tabs: All (default), Moderation, Errors, Announcements

### Flashing / Activity Rules

Each tab maintains:
- `unread_count` — incremented on message arrival when tab is not focused
- `has_mention` — (future) set when player name appears in message
- `flash_state` — boolean for animation toggle
- `last_activity_time` — timestamp of most recent message

When a message arrives for a tab that is **not** focused:
1. Increment `unread_count`
2. Start flashing that tab (alternating color every ~500ms or pulsing alpha)

When user clicks/focuses a tab:
1. `unread_count = 0`
2. Stop flashing

For DM sub-tabs, the **Whisper parent tab** also flashes if any child has unread.

---

## 2. Stream Identity Model

Each message belongs to a **stream** identified by:

```
stream_type  = Global | Local | Party | Guild | Whisper | System | AdminConsole
stream_key   = (depends on type)
  - Local:   solar_system_id
  - Party:   party_id
  - Guild:   guild_id
  - Whisper: thread_id = (min(sender_char_id, peer_char_id), max(...))
  - System / Global: 0
```

Each stream has a **monotonic `server_seq`** for guaranteed ordered display.

---

## 3. Message Structure

### ChatMessage (Server → Client)

| Field | Type | Description |
|-------|------|-------------|
| `stream_type` | enum | Channel type |
| `stream_key` | string | Stream identifier |
| `server_seq` | uint64 | Monotonic sequence per stream |
| `server_msg_id` | string | Unique message ID |
| `timestamp_utc_ms` | uint64 | Server timestamp |
| `sender_player_id` | string | Authoritative player ID |
| `sender_character_id` | string | Authoritative character ID |
| `sender_character_name` | string | Display name (server-attached) |
| `text` | string | UTF-8 message body (may include emoji/shortcodes) |
| `flags` | uint32 | Bitfield: system, moderation, broadcast, etc. |
| `system_category` | enum | (System only) All, Moderation, Errors, Announcements |

### ChatSend (Client → Server)

| Field | Type | Description |
|-------|------|-------------|
| `channel` | enum | Target channel type |
| `target_character_id` | string | Required for Whisper |
| `client_msg_id` | string | Client-assigned UUID for ack correlation |
| `text` | string | UTF-8 message body |

### ChatSendResult (Server → Client, ack to sender)

| Field | Type | Description |
|-------|------|-------------|
| `client_msg_id` | string | Echo of client's ID |
| `success` | bool | Whether message was accepted |
| `error_code` | enum | `none`, `rate_limited`, `muted`, `too_long`, `invalid_channel` |
| `server_msg_id` | string | Assigned server ID (if success) |

### ChatHistoryChunk (Server → Client, on join/reconnect)

| Field | Type | Description |
|-------|------|-------------|
| `stream_type` | enum | Channel type |
| `stream_key` | string | Stream identifier |
| `messages` | array | List of ChatMessage objects |

### AdminCommandSend (Client → Server)

| Field | Type | Description |
|-------|------|-------------|
| `command_text` | string | Admin command to execute |
| `client_cmd_id` | string | Correlation ID |

### AdminCommandResult (Server → Client)

| Field | Type | Description |
|-------|------|-------------|
| `client_cmd_id` | string | Correlation ID |
| `output` | string | Command output text |
| `success` | bool | Whether command succeeded |

---

## 4. Emoji Support

- **Unicode emoji:** UTF-8 passthrough; font must support rendering
- **Custom shortcodes:** `:heart:`, `:pog:`, etc. — client parses and renders inline
- **Server validation:** Enforce max length on UTF-8 bytes (512 max)
- **v1:** Render shortcodes as colored text; add sprite substitution later

---

## 5. Transport & Reliability

- Server assigns increasing `server_seq` per stream
- Client displays messages strictly in `server_seq` order, buffering gaps
- TCP transport provides reliable ordered delivery at the socket level
- Application-level `server_seq` provides safety net for display ordering

---

## 6. Rate Limiting & Moderation

| Rule | Value |
|------|-------|
| Max message length | 512 UTF-8 bytes |
| Rate limit | 3 messages / 5 seconds per player per channel |
| Mute | Server enforces; muted players get `ChatSendResult.error_code = muted` |
| Ban | Banned players cannot join channels |
| Profanity filter | Optional server-side; v1 can skip |

---

## 7. Identity

- **Display name:** Character name (server-attached, authoritative)
- **Signed with:** `player_id` + `character_id` (server-attached)
- **Clicking a player name:** Opens character info page / friend request

---

## 8. Persistence

| Scope | Storage | Retention |
|-------|---------|-----------|
| Client | In-memory ring buffer per tab | 1000 messages |
| Server | Full logs per stream | 90 days |
| Whispers | Server-persisted | 90 days |

On join/reconnect, client requests last N messages for:
- Global, System, Local (current system)
- Party/Guild (if member)
- Recent DM threads

---

## 9. Server Console (Admin Tab)

- `AdminCommandSend`: client → server (`command_text`)
- `AdminCommandResult`: server → client (`output`, `success`)
- Permission gate: server checks admin role before executing
- Not a normal chat channel — separate message class

---

## 10. Implementation Architecture

### Three Layers

1. **Chat UI** (client-only): Tab bar, message view, input box, scrollback
2. **Chat Domain** (shared logic): Message struct, channel enum, formatting, spam throttle
3. **Chat Transport** (client ↔ server): Packet definitions, send queue, receive handlers

### Server-Side Files

| File | Purpose |
|------|---------|
| `cpp_server/include/handlers/chat_handler.h` | `IMessageHandler` for CHAT messages |
| `cpp_server/src/handlers/chat_handler.cpp` | Route/validate/broadcast chat messages |
| `cpp_server/include/systems/chat_system.h` | Existing ECS system (channels, members, moderation) |
| `cpp_server/include/components/social_components.h` | `ChatChannel` component (expanded) |

### Client-Side Files

| File | Purpose |
|------|---------|
| `cpp_client/include/chat/chat_service.h` | Domain API: send/receive, ring buffer, stream mgmt |
| `cpp_client/src/chat/chat_service.cpp` | Implementation |
| `cpp_client/include/ui/atlas/chat_widget.h` | Atlas UI chat panel with tabs |
| `cpp_client/src/ui/atlas/chat_widget.cpp` | Implementation |

### Protocol Files

| File | Purpose |
|------|---------|
| `cpp_server/include/network/protocol_handler.h` | Extended with chat-specific message types |

---

## 11. Protocol Message Types (Extended)

New additions to `MessageType` enum:

```
CHAT_SEND           // Client → Server: send a chat message
CHAT_MESSAGE        // Server → Client: broadcast a chat message
CHAT_SEND_RESULT    // Server → Client: ack/error for sent message
CHAT_HISTORY        // Server → Client: history chunk on join
CHAT_JOIN           // Client → Server: join a channel
CHAT_LEAVE          // Client → Server: leave a channel
ADMIN_COMMAND_SEND  // Client → Server: admin console command
ADMIN_COMMAND_RESULT // Server → Client: admin command response
```
