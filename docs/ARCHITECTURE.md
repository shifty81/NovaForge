# NovaForge Architecture

## Overview
NovaForge is a .NET 8 C# game engine scaffold with the following layers:

## Projects

### NovaForge.Core
Foundation utilities: logging, math helpers, deterministic RNG, and an event bus.
Game events (`VoxelMinedEvent`, `SalvageClaimedEvent`) are defined here and published via `EventBus`.
`PerformanceMetrics` (in `Diagnostics/`) tracks per-frame FPS, average frame time, chunks rendered, and triangles drawn; callers call `BeginFrame`/`EndFrame`/`Sample` each frame.

### NovaForge.Data
Data models (RoomDefinition, ResourceDefinition, SalvageNodeDefinition) and a JSON DataLoader.

### NovaForge.ProcGen
Procedural interior generation using seeded deterministic RNG. Produces InteriorLayout with RoomInstance and SalvageNodeInstance.
`WorldBuilder` stamps an InteriorLayout into a ChunkManager as voxel geometry (floor=type 1, wall=type 2, ore/salvage=type 3, doorways).

### NovaForge.Voxels
Chunk-based voxel storage (`VoxelChunk`, `ChunkManager`), greedy meshing (`VoxelMesher`), and DDA raycasting (`VoxelRaycaster`).
- `VoxelChunk.IsDirty` tracks modifications; `ClearDirty()` resets the flag; `ResetEdits()` clears the edit log (used after world gen).
- `ChunkManager.GetDirtyChunks()` / `ClearAllDirty()` / `ResetAllEdits()` drive incremental mesh rebuilds and clean save deltas.
- The greedy mesher emits 7 floats per vertex: `[x, y, z, nx, ny, nz, voxelType]` (`MeshData.FloatsPerVertex = 7`).

### NovaForge.Render
OpenTK 4 rendering: ShaderProgram (GLSL 330), GpuMesh (VAO/VBO/EBO), Camera, and NovaForgeWindow.
- Vertex layout: position (loc 0), normal (loc 1), voxelType (loc 2).
- Fragment shader maps voxel type to base colour: 1=floor (concrete), 2=wall (metal), 3=ore (orange).
- `GpuMesh.TriangleCount` exposes the triangle count of the last uploaded mesh for performance tracking.

### NovaForge.Game
Main game loop: integrates all layers. Handles input, mining, salvage scanning (deterministic yield), save/load (WorldDelta JSON), and multi-chunk rendering.
- After WorldBuilder.StampLayout, calls `ResetAllEdits()` so saves only persist player-driven changes.
- `RebuildDirtyMeshes()` rebuilds only chunks modified since last clear, keeping per-frame cost low.
- **Player collision**: per-axis AABB check (0.6 m wide × 1.8 m tall) against solid voxels before committing each frame's movement, enabling sliding along walls.
- **Performance overlay**: `PerformanceMetrics` drives the window title (FPS, frame time, chunk count, triangle count), updated once per second.

### NovaForge.Tests
xUnit tests: deterministic generation, voxel delta restoration, greedy meshing (vertex stride, type encoding), world stamping (floor/wall types, dirty flag, reset), `VoxelChunk` dirty-tracking lifecycle, and `PerformanceMetrics` (FPS accuracy, chunk/triangle counters, sampling interval).

## Data Flow
```
DataLoader -> InteriorGenerator -> InteriorLayout
InteriorLayout -> WorldBuilder -> ChunkManager (floor=1, wall=2, ore=3, doorways)
ChunkManager.ResetAllEdits() -- clear gen edits so save = player delta only
ChunkManager<dirty> -> VoxelMesher (greedy, 7 floats/vert) -> GpuMesh -> ShaderProgram -> Display
VoxelRaycaster -> ChunkManager (mine, marks chunk dirty) -> EventBus (VoxelMinedEvent)
Scan -> SalvageNodeInstance -> DeterministicRng (yield) -> Inventory -> EventBus (SalvageClaimedEvent)
SaveManager -> WorldDelta (JSON, player delta only) -> ChunkManager / InteriorLayout
```
