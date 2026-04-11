# NovaForge Architecture

## Overview
NovaForge is a .NET 8 C# game engine scaffold with the following layers:

## Projects

### NovaForge.Core
Foundation utilities: logging, math helpers, deterministic RNG, and an event bus.
Game events (`VoxelMinedEvent`, `SalvageClaimedEvent`) are defined here and published via `EventBus`.

### NovaForge.Data
Data models (RoomDefinition, ResourceDefinition, SalvageNodeDefinition) and a JSON DataLoader.

### NovaForge.ProcGen
Procedural interior generation using seeded deterministic RNG. Produces InteriorLayout with RoomInstance and SalvageNodeInstance.
`WorldBuilder` stamps an InteriorLayout into a ChunkManager as voxel geometry (floors, walls, doorways).

### NovaForge.Voxels
Chunk-based voxel storage (VoxelChunk, ChunkManager), greedy meshing (VoxelMesher), and DDA raycasting (VoxelRaycaster).
The greedy mesher merges adjacent co-planar faces of the same type into larger quads, reducing triangle count.

### NovaForge.Render
OpenTK 4 rendering: ShaderProgram (GLSL 330), GpuMesh (VAO/VBO/EBO), Camera, and NovaForgeWindow.

### NovaForge.Game
Main game loop: integrates all layers. Handles input, mining, salvage scanning (with deterministic yield per ResourceDefinition), save/load (WorldDelta JSON), and multi-chunk rendering.

### NovaForge.Tests
xUnit tests for deterministic generation, voxel delta restoration, greedy meshing behaviour, and world stamping.

## Data Flow
```
DataLoader -> InteriorGenerator -> InteriorLayout
InteriorLayout -> WorldBuilder -> ChunkManager (voxel rooms + doorways)
ChunkManager <-> VoxelMesher (greedy) -> GpuMesh -> ShaderProgram -> Display (all chunks)
VoxelRaycaster -> ChunkManager (mine) -> EventBus (VoxelMinedEvent)
Scan -> SalvageNodeInstance -> DeterministicRng (yield) -> Inventory -> EventBus (SalvageClaimedEvent)
SaveManager -> WorldDelta (JSON) -> ChunkManager / InteriorLayout
```
