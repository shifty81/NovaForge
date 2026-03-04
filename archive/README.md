# Archive

This directory contains legacy and deprecated files from the Atlas project's earlier development stages. All code and documents here are kept for historical reference only.

## Contents

### `python/` — Legacy Python Prototype
The original Python implementation before the project migrated to C++.
- `client/` — Original text/GUI client (pygame-based)
- `server/` — Original Python game server
- `engine/` — Core game engine (Python)
- `client_3d/` — 3D client prototype (Panda3D-based)
- `tests/` — Python test files
- `tests_suite/` — Additional test suite
- `demos/` — Demo and showcase scripts
- Build scripts and dependencies

### `legacy/` — Deprecated Project Files
- `eve baseline.rtf` — Original design baseline document
- `scopefor projectgui.rtf` — Early GUI scope document
- `scopeforprojectsofar.rtf` — Historical project scope notes
- `eve_online_manual.pdf` — External reference manual
- `rename_mapping.py` — Content renaming utility (ships, factions, etc.)
- `SESSION_SUMMARY.md` — Development session summary
- `QUICKSTART_VISUALS.md` — Visual quick-start guide (superseded by main README)

### `chat_logs/` — Raw Conversation Transcripts
Unstructured AI/human conversation logs used during design and development.
Design decisions have been extracted into proper documentation under `docs/design/`.
- `chatlog1.txt`, `newupload.txt`, `new123`, `newstuffforPCG` — Development discussions
- `atlas plans for future.txt`, `morepcg.txt` — Planning notes
- `atlasUI.rtf` — UI design conversation
- `pcg_pipeline_design_notes.txt` — PCG pipeline brainstorming

### `editornewfeat_chat_transcript.md` — Editor Feature Planning
Raw AI/human conversation about editor tool layer design, ToolingLayer, EditorCommandBus,
RuntimeBootstrap, and extended editor features. Extracted into:
- [`docs/design/editor_tool_layer.md`](../docs/design/editor_tool_layer.md)
- [`docs/design/extended_tooling_features.md`](../docs/design/extended_tooling_features.md)
- [`docs/EDITOR_TOOLS.md`](../docs/EDITOR_TOOLS.md)

### `features_chat_transcript.md` — Feature Planning
Raw conversation covering cinematic warp, fleet AI, ship generation, and universe design.

## Active Code

The active Atlas implementation is in:
- `engine/` — Atlas Engine (C++ static library)
- `editor/` — Atlas Editor with 17 panels and dock layout
- `cpp_client/` — C++ OpenGL game client with Atlas UI and 32 embedded editor tools
- `cpp_server/` — C++ dedicated game server with PCG pipeline
- `data/` — Game data (JSON configs)
- `atlas_tests/` — Unit tests (374+ assertions)
