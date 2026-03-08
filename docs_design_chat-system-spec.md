# NovaForge — Chat System (Full Design Spec)

Status: Draft v1  
Date: 2026-03-08  
Scope: **Implement full in-game Chat** in `shifty81/NovaForge` with **custom UI** (no ImGui), server-authoritative enforcement, guaranteed delivery, strict in-order display, emojis, tabs w/ activity flashing, and persistence (90-day retention).

---

## 0) Summary of requirements (locked)

### Chat types / tabs
- Global
- Local (**Solar System** scope)
- Party
- Guild
- Whisper (**DM sub-tabs per peer**)
- System (**parent + category sub-tabs**)
- Server tab (admin console style: commands + output)

### UI behavior
- Tabbed UI; tabs **flash / highlight on activity** when not focused
- Unread counters per tab and per DM/system sub-tab
- Click sender name to open DM tab and view character page (character page is out-of-scope here, but chat must provide the IDs needed)

### Transport / authority
- Server-authoritative:
  - rate limits
  - spam checks
  - mute/ban enforcement
  - identity attachment (player ID, character ID, display name)
- Ordering: **no out-of-order display**
- Delivery: **guaranteed**
- Works whether underlying transport ends up being TCP, UDP, or mixed (protocol supports ACK-based recovery)

### Emojis
- Supports both:
  - Unicode emoji (UTF-8)
  - shortcodes (e.g. `:heart:`), allowing future sprite mapping

### Persistence
- Client: keep last **1000** messages per stream locally (in-memory; optionally persisted to disk later)
- Server: store full logs with **90-day retention**, including whispers (DMs)
- Client can request recent history on connect/reconnect/open-tab

---

## 1) Concepts & terminology

### 1.1 Channel vs Stream
- **Channel** is the semantic type: Global/Local/Party/Guild/Whisper/System/ServerConsole.
- **Stream** is a concrete ordered message sequence with its own monotonic server sequence number.
  - Example streams:
    - Global stream
    - Local stream for solarSystemId=42
    - Whisper thread stream for (charA,charB)
    - System stream (single stream, with category filtering)

### 1.2 Strict ordering
- A client must **render messages per stream** strictly in `server_seq` order.
- If message `server_seq = N+1` arrives before `N`, the client buffers `N+1` until `N` arrives (bounded wait + recovery via history request/ACK mechanism).

---

## 2) UI / UX Specification

### 2.1 Top-level tabs
1. Global
2. Local (Solar System)
3. Party
4. Guild
5. Whisper
6. System
7. Server (admin console)

### 2.2 Whisper DM sub-tabs
- Under **Whisper**:
  - Each DM peer is a sub-tab named by the peer’s character name.
  - There is an optional "Whisper: All" sub-tab (optional; can be added later).
- Opening a DM:
  - clicking a player name -> "Whisper" creates/focuses the DM sub-tab
  - typing command `/w <name> <msg>` creates/focuses DM sub-tab

### 2.3 System category sub-tabs
Under **System**, show:
- System: All (always includes every system message)
- System: Announcements
- System: Moderation
- System: Errors

Routing rules:
- Every system message appears in **System: All**
- Additionally appears in the category sub-tab matching `system_category`

### 2.4 Activity flashing + unread counts
Each (sub)tab maintains:
- `unread_count`
- `has_activity` (or `flash_until_time`)
- optional `has_mention` (future)

Rules:
- If a message arrives for a tab/sub-tab that is not currently focused:
  - increment unread_count
  - mark activity for flashing/highlight
- When a tab/sub-tab becomes focused:
  - reset that tab’s unread_count to 0
  - clear activity flashing for that tab/sub-tab
- Parent tabs aggregate:
  - Whisper parent shows unread sum of DM sub-tabs
  - System parent shows unread sum of category sub-tabs
  - Parent flashes if any child has activity

Visual recommendation (non-annoying):
- Always show unread badge.
- Flash/highlight for first N seconds (e.g., 5s) after activity, then remain highlighted until read.

### 2.5 Input focus behavior
- Press **Enter**:
  - if chat is closed -> open chat and focus input
  - if chat is open and input has text -> send (or add newline if you support multi-line)
- Press **Esc**:
  - blur/unfocus input (closes chat input mode)
- While input focused:
  - gameplay keybinds should not trigger (chat consumes input)
- Support command parsing:
  - `/g`, `/l`, `/p`, `/guild` etc.
  - `/w <name> <text>`
  - `/r <text>` reply to last whisper (optional v1)

### 2.6 Text rendering
- Must support UTF-8 including Unicode emoji.
- Additionally parse shortcodes `:name:` as inline tokens:
  - v1: render as colored text token if sprites aren’t implemented
  - v2: map to sprite atlas icons if desired
- Limits:
  - Max chars: pick a v1 number (e.g., 512 bytes UTF-8)
  - Must prevent UI overflow (wrapping, truncation in list view with full copy on hover if desired)

---

## 3) Identity & data carried by each message

### 3.1 Identity fields (authoritative from server)
Each delivered chat message includes:
- `sender_player_id` (unique)
- `sender_character_id` (unique)
- `sender_character_name` (display)
- `timestamp_utc_ms` (server time)

Client must treat these as **read-only** and not user-controlled.

### 3.2 Character page integration
Chat must expose:
- `sender_character_id` and `sender_player_id`
so the UI can open the character page when clicked (out of scope here).

---

## 4) Scope rules per channel

### 4.1 Global
- Delivered to all connected players who have access (normal).

### 4.2 Local (Solar System)
- Delivered to all players currently in the same `solar_system_id`.
- The server determines `solar_system_id` from player state (not from the client’s message payload).

### 4.3 Party
- Delivered to members of the sender’s party.
- If sender is not in a party, server returns error.

### 4.4 Guild
- Delivered to members of the sender’s guild.
- If sender is not in a guild, server returns error.

### 4.5 Whisper (DM)
- Delivered only to sender and target peer.
- Persisted server-side (90 days).
- Delivery should fail with explicit error if:
  - target not found
  - target offline (depending on design; optional: still store for later delivery)
  - sender muted
  - sender blocked by target (if you implement blocks later)

### 4.6 System
- Server-generated only.
- Used for:
  - announcements
  - moderation notices (mute/ban actions, warnings)
  - errors (rate-limit feedback, delivery failures)
- Client should not send System messages.

### 4.7 Server (admin console)
- Only for authorized users (role/permission check).
- Supports:
  - sending commands
  - receiving server output
- Not a normal player chat channel.

---

## 5) Reliability, ordering, and synchronization

### 5.1 Server sequence numbers
For each stream, the server maintains:
- `server_seq` (uint64) monotonically increasing per stream.

Each message delivered includes:
- `stream_id` (or `stream_type + stream_key`)
- `server_seq`

The client:
- tracks `expected_next_seq` per stream
- buffers future messages until gaps are filled
- requests missing messages if a gap persists beyond a short timeout

### 5.2 ACK model (transport-independent)
Client sends periodic ACKs per stream:
- `highest_contiguous_server_seq_received` for that stream

Server retains a bounded resend/history buffer to ensure delivery (plus long-term persistence for 90 days).

### 5.3 Handling disconnects/reconnects
On reconnect:
- client sends `ChatSyncRequest` containing last known `highest_contiguous_server_seq_received` for key streams
- server replies with `ChatHistoryChunk` (or multiple chunks) to fill gaps

### 5.4 Guaranteed delivery definition (practical)
Guaranteed delivery here means:
- If the client is connected and authenticated, and the server accepts the message,
  then the message will eventually appear in the client’s stream (no silent drops),
  assuming the connection remains usable (or can reconnect and resync).

---

## 6) Message protocol (suggested)

> Exact binary/JSON encoding depends on NovaForge’s current networking layer. The *fields and semantics* should stay consistent.

### 6.1 Common types
- `PlayerId` (uint64)
- `CharacterId` (uint64)
- `MessageId` (uint64 or UUID)
- `TimestampUtcMs` (uint64)
- `StreamType` enum
- `SystemCategory` enum
- `StreamKey` (uint64; meaning depends on stream type)

### 6.2 Client -> Server messages

#### `ChatSend`
Fields:
- `client_msg_id` (unique per client send)
- `channel` (Global | Local | Party | Guild | Whisper)
- `target_character_id` (required for Whisper; otherwise 0)
- `text` (UTF-8)
Notes:
- Client never specifies `sender_*`
- Client never specifies solarSystemId/partyId/guildId; server derives from authoritative state

#### `ChatAck`
Fields:
- `stream_type`
- `stream_key`
- `highest_contiguous_server_seq`

#### `ChatHistoryRequest`
Fields:
- `stream_type`
- `stream_key`
- Either:
  - `after_server_seq` (request newer), OR
  - `range_start` + `range_end`
- `max_messages`

#### `AdminCommandSend`
Fields:
- `client_cmd_id`
- `command_text`

### 6.3 Server -> Client messages

#### `ChatMessage`
Fields:
- `stream_type`
- `stream_key`
- `server_seq`
- `server_msg_id`
- `timestamp_utc_ms`
- `sender_player_id`
- `sender_character_id`
- `sender_character_name`
- `text`
- if `stream_type == System`: `system_category`

#### `ChatSendResult`
Fields:
- `client_msg_id`
- `result` (Ok | Error)
- `error_code` (Muted | RateLimited | TooLong | NotInParty | NotInGuild | TargetNotFound | Unauthorized | Unknown)
- `human_message` (optional; often also delivered as a System:Errors message)

#### `ChatHistoryChunk`
Fields:
- `stream_type`
- `stream_key`
- `messages[]` (array of ChatMessage, ordered by server_seq ascending)

#### `AdminCommandResult`
Fields:
- `client_cmd_id`
- `timestamp_utc_ms`
- `stdout`
- `stderr`
- `exit_code`

---

## 7) Server responsibilities

### 7.1 Validation
- length limits (bytes)
- allowed characters (UTF-8 validation)
- rate limiting (per player, per channel)
- mute/ban enforcement
- permissions for AdminConsole

### 7.2 Routing
- Global -> all
- Local -> all in same solar system
- Party/Guild -> members
- Whisper -> sender + target (and optional offline storage + later delivery)

### 7.3 Persistence (90-day retention)
Store messages with:
- stream identifiers
- server_seq
- timestamp
- sender identifiers
- text
- system category (if any)

Retention:
- periodic cleanup job deletes records older than 90 days

---

## 8) Client responsibilities

### 8.1 ChatService (domain)
Expose:
- `SendGlobal(text)`
- `SendLocal(text)`
- `SendParty(text)`
- `SendGuild(text)`
- `SendWhisper(targetCharacterId, text)`
- `OpenDM(targetCharacterId)`
- `RequestHistory(stream, after_seq, max)`
- `OnChatMessageReceived` event/callback subscription

### 8.2 ChatStore (state)
Maintain:
- ring buffer per stream (cap 1000)
- per-stream ordering state:
  - `expected_next_seq`
  - out-of-order buffer map (seq -> message)
- unread/flash state per tab/sub-tab

### 8.3 UI widgets
- Subscribe to store events and redraw when:
  - new message added
  - unread counts changed
  - focus/tab changes

---

## 9) Emoji handling details

### 9.1 Unicode emoji
- Send as UTF-8.
- Ensure font fallback supports emoji (implementation detail of your custom UI text renderer).

### 9.2 Shortcodes
- Client parses patterns like `:heart:` into tokens.
- v1: render token as `:heart:` but styled (color/background)
- v2: map to sprite index (requires a registry of shortcode -> sprite)

Server-side:
- does not need to interpret; can store raw text
- optional: validate allow-list of shortcodes later

---

## 10) MVP milestones (recommended implementation order)

### Milestone 1: Protocol + basic delivery
- Define chat protocol messages and register them in net layer
- Server: accept Global chat, broadcast to all
- Client: display Global chat list + send input

### Milestone 2: Streams + ordering + ACK
- Add server_seq
- Add buffering and strict ordering on client
- Add ACKs + history request on reconnect

### Milestone 3: Tabs + flashing + unread counts
- Implement tab UI and activity indicators

### Milestone 4: Local/Party/Guild routing
- Local uses solar system membership
- Party/Guild uses membership state

### Milestone 5: Whispers with DM sub-tabs + persistence
- DM routing + per-thread stream IDs
- Persist DM logs server-side 90 days
- History request support

### Milestone 6: System categories + admin console tab
- System:All/Announcements/Moderation/Errors sub-tabs
- Admin command send/result with permission checks

---

## 11) Open decisions (optional; can be set later)
- Maximum message size (suggest 512 bytes UTF-8 to start)
- Whether whispers to offline users deliver later (store-and-forward) or error out
- Mention/highlight rules (`@name`) and notification sounds
- Profanity filtering approach

---