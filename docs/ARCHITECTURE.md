# NovaForge Architecture

## Overview
NovaForge is a .NET 8 C# game engine scaffold with the following layers:

## Projects

### NovaForge.Core
Foundation utilities: logging, math helpers, deterministic RNG, and an event bus.

### NovaForge.Data
Data models (RoomDefinition, ResourceDefinition, SalvageNodeDefinition) and a JSON DataLoader.

### NovaForge.ProcGen
Procedural interior generation using seeded deterministic RNG. Produces InteriorLayout with RoomInstance and SalvageNodeInstance.

### NovaForge.Voxels
Chunk-based voxel storage (VoxelChunk, ChunkManager), greedy meshing (VoxelMesher), and DDA raycasting (VoxelRaycaster).

### NovaForge.Render
OpenTK 4 rendering: ShaderProgram (GLSL 330), GpuMesh (VAO/VBO/EBO), Camera, and NovaForgeWindow.

### NovaForge.Game
Main game loop: integrates all layers. Handles input, mining, salvage scanning, save/load (WorldDelta JSON).

### NovaForge.Tests
xUnit tests for deterministic generation and voxel delta restoration.

## Data Flow
```
DataLoader -> InteriorGenerator -> InteriorLayout
ChunkManager <-> VoxelMesher -> GpuMesh -> ShaderProgram -> Display
VoxelRaycaster -> ChunkManager (mine)
SaveManager -> WorldDelta (JSON) -> ChunkManager / InteriorLayout
```
