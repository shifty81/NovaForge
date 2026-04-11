using System;
using System.Collections.Generic;
using NovaForge.Core.Diagnostics;
using NovaForge.Core.Events;
using NovaForge.Core.Logging;
using NovaForge.Core.Math;
using NovaForge.Core.Rng;
using NovaForge.Data;
using NovaForge.Data.Models;
using NovaForge.ProcGen.Interior;
using NovaForge.Render;
using NovaForge.Render.Mesh;
using NovaForge.Render.Shaders;
using NovaForge.Voxels.Chunk;
using NovaForge.Voxels.Meshing;
using NovaForge.Voxels.Raycast;
using NovaForge.Game.SaveLoad;
using OpenTK.Graphics.OpenGL4;
using OpenTK.Mathematics;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.GraphicsLibraryFramework;

namespace NovaForge.Game
{
    public class Game
    {
        private NovaForgeWindow _window;
        private Camera _camera;
        private ShaderProgram _shader;
        private readonly Dictionary<(int, int, int), GpuMesh> _gpuMeshes = new Dictionary<(int, int, int), GpuMesh>();
        private ChunkManager _chunkManager;
        private InteriorLayout _layout;
        private Inventory _inventory;
        private SaveManager _saveManager;
        private VoxelMesher _mesher;
        private VoxelRaycaster _raycaster;
        private EventBus _events;
        private PerformanceMetrics _metrics;

        private long _seed = 42L;
        private bool _firstMouseMove = true;
        private float _lastMouseX, _lastMouseY;
        private const float MoveSpeed = 10f;

        // Gravity and jump physics.
        private float _velocityY = 0f;
        private const float Gravity = -20f;
        private const float JumpSpeed = 7f;
        private const float TerminalVelocity = -30f;

        // Player AABB half-extents and eye-level offset.
        private const float PlayerRadius = 0.3f;
        private const float PlayerEyeHeight = 1.6f;
        private const float PlayerCrownOffset = 0.2f;
        // Fraction of eye-height used for the mid-body collision check (between feet and eyes).
        private const float PlayerMidpointFraction = 0.5f;
        // Distance below feet used to detect whether the player is standing on solid ground.
        private const float GroundCheckDistance = 0.05f;

        // Maps voxel type to the resource ID awarded when mining that block.
        private static readonly string[] VoxelTypeToResource = { "stone", "stone", "iron_scrap", "rare_ore" };

        private List<RoomDefinition> _roomDefs;
        private List<SalvageNodeDefinition> _nodeDefs;
        private List<ResourceDefinition> _resourceDefs;

        public void Run()
        {
            _window = new NovaForgeWindow(1280, 720, "NovaForge");
            _window.UpdateCallback = OnUpdate;
            _window.RenderCallback = OnRender;
            _window.Load += OnLoad;
            _window.Run();
        }

        private void OnLoad()
        {
            Logger.Log("NovaForge starting...");

            GL.Enable(EnableCap.DepthTest);
            GL.ClearColor(0.1f, 0.1f, 0.15f, 1.0f);

            _camera = new Camera { Position = new Vector3(16, 8, 16) };
            _shader = new ShaderProgram(ShaderProgram.VertexSource, ShaderProgram.FragmentSource);
            _mesher = new VoxelMesher();
            _raycaster = new VoxelRaycaster();
            _chunkManager = new ChunkManager();
            _inventory = new Inventory();
            _saveManager = new SaveManager();
            _metrics = new PerformanceMetrics();

            _events = new EventBus();
            _events.Subscribe<VoxelMinedEvent>(e =>
                Logger.Log($"[Event] Voxel mined at {e.Position} (type {e.OldType})"));
            _events.Subscribe<SalvageClaimedEvent>(e =>
                Logger.Log($"[Event] Salvage claimed: {e.Amount}x {e.ResourceId} from node {e.NodeId}"));

            string dataPath = System.IO.Path.Combine(AppContext.BaseDirectory, "data");
            if (!System.IO.Directory.Exists(dataPath))
                dataPath = System.IO.Path.GetFullPath(System.IO.Path.Combine(AppContext.BaseDirectory, "../../../../../data"));

            _roomDefs = DataLoader.LoadRooms(dataPath);
            _resourceDefs = DataLoader.LoadResources(dataPath);
            _nodeDefs = DataLoader.LoadSalvageNodes(dataPath);

            var generator = new InteriorGenerator();
            _layout = generator.Generate(_seed, _roomDefs, _nodeDefs);
            Logger.Log($"Generated {_layout.Rooms.Count} rooms.");

            WorldBuilder.StampLayout(_layout, _chunkManager);

            // Stamp is deterministic — reset edits so saves only persist player changes.
            _chunkManager.ResetAllEdits();

            RebuildAllMeshes();

            _window.CursorState = CursorState.Grabbed;
            Logger.Log("NovaForge loaded. WASD=move, Space=jump, mouse=look, E=scan, LMB=mine, F5=save, F9=load");
        }

        private void RebuildAllMeshes()
        {
            foreach (var kv in _chunkManager.GetChunks())
                RebuildChunkMesh(kv.Key, kv.Value);
            _chunkManager.ClearAllDirty();
        }

        private void RebuildDirtyMeshes()
        {
            foreach (var kv in _chunkManager.GetDirtyChunks())
                RebuildChunkMesh(kv.Key, kv.Value);
            _chunkManager.ClearAllDirty();
        }

        private void RebuildChunkMesh((int, int, int) key, NovaForge.Voxels.Chunk.VoxelChunk chunk)
        {
            var meshData = _mesher.GenerateMesh(chunk);
            if (_gpuMeshes.TryGetValue(key, out var existing))
                existing.Update(meshData);
            else
                _gpuMeshes[key] = new GpuMesh(meshData);
        }

        private void OnUpdate(double dt)
        {
            var kb = _window.KeyboardState;
            var ms = _window.MouseState;
            float fdt = (float)dt;

            if (kb.IsKeyDown(Keys.Escape)) _window.Close();

            // Capture position before movement so we can resolve collision per-axis.
            var oldPos = _camera.Position;

            if (kb.IsKeyDown(Keys.W)) _camera.MoveForward(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.S)) _camera.MoveBack(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.A)) _camera.MoveLeft(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.D)) _camera.MoveRight(MoveSpeed, fdt);

            // Apply gravity and move camera vertically.
            _velocityY = Math.Max(_velocityY + Gravity * fdt, TerminalVelocity);
            _camera.Position = new Vector3(_camera.Position.X, _camera.Position.Y + _velocityY * fdt, _camera.Position.Z);

            // Per-axis AABB collision against solid voxels.
            var newPos = _camera.Position;
            float resolvedX = newPos.X;
            float resolvedY = newPos.Y;
            float resolvedZ = newPos.Z;

            if (IsPlayerPositionBlocked(new Vector3(newPos.X, oldPos.Y, oldPos.Z)))
                resolvedX = oldPos.X;
            if (IsPlayerPositionBlocked(new Vector3(resolvedX, oldPos.Y, newPos.Z)))
                resolvedZ = oldPos.Z;

            bool yBlocked = IsPlayerPositionBlocked(new Vector3(resolvedX, newPos.Y, resolvedZ));
            if (yBlocked)
            {
                resolvedY = oldPos.Y;
                _velocityY = 0f;
            }

            _camera.Position = new Vector3(resolvedX, resolvedY, resolvedZ);

            // Jump when grounded (Y was blocked by floor beneath).
            bool isGrounded = yBlocked || IsPlayerPositionBlocked(new Vector3(resolvedX, resolvedY - GroundCheckDistance, resolvedZ));
            if (isGrounded && kb.IsKeyPressed(Keys.Space))
                _velocityY = JumpSpeed;

            float mx = ms.X, my = ms.Y;
            if (_firstMouseMove) { _lastMouseX = mx; _lastMouseY = my; _firstMouseMove = false; }
            float dx = mx - _lastMouseX;
            float dy = my - _lastMouseY;
            _lastMouseX = mx; _lastMouseY = my;
            _camera.ProcessMouseMovement(dx, dy);

            if (kb.IsKeyPressed(Keys.E)) Scan();
            if (kb.IsKeyPressed(Keys.F5)) _saveManager.Save(_seed, _chunkManager, _layout, _inventory);
            if (kb.IsKeyPressed(Keys.F9)) LoadGame();
            if (ms.IsButtonPressed(MouseButton.Left)) Mine();

            // Update performance metrics and refresh window title once per second.
            _metrics.EndFrame(dt);
            if (_metrics.Sample())
                _window.Title = $"NovaForge  |  {_metrics}";
        }

        /// <summary>
        /// Returns true if the player AABB centred at <paramref name="pos"/> (camera/eye position)
        /// overlaps at least one solid voxel. Checks a 2×3×2 grid of AABB corners.
        /// </summary>
        private bool IsPlayerPositionBlocked(Vector3 pos)
        {
            float[] xOff = { -PlayerRadius, PlayerRadius };
            float[] zOff = { -PlayerRadius, PlayerRadius };
            float[] yOff = { -PlayerEyeHeight, -PlayerEyeHeight * PlayerMidpointFraction, PlayerCrownOffset };

            foreach (float yo in yOff)
            foreach (float xo in xOff)
            foreach (float zo in zOff)
            {
                int vx = (int)Math.Floor(pos.X + xo);
                int vy = (int)Math.Floor(pos.Y + yo);
                int vz = (int)Math.Floor(pos.Z + zo);
                if (_chunkManager.GetVoxel(vx, vy, vz) != 0)
                    return true;
            }
            return false;
        }

        private void Scan()
        {
            Logger.Log("Scanning for salvage nodes...");
            var camPos = _camera.Position;
            float scanRadius = 5f;

            foreach (var room in _layout.Rooms)
            {
                foreach (var node in room.SalvageNodes)
                {
                    if (node.IsClaimed) continue;
                    float ddx = node.LocalPosition.X + room.Position.X * WorldBuilder.Scale - camPos.X;
                    float ddz = node.LocalPosition.Z + room.Position.Y * WorldBuilder.Scale - camPos.Z;
                    float dist = (float)System.Math.Sqrt(ddx * ddx + ddz * ddz);
                    if (dist <= scanRadius)
                    {
                        ClaimSalvageNode(room, node);
                        return;
                    }
                }
            }
            Logger.Log("No salvage nodes in range.");
        }

        private void ClaimSalvageNode(ProcGen.Interior.RoomInstance room, ProcGen.Interior.SalvageNodeInstance node)
        {
            node.IsClaimed = true;

            var nodeDef = _nodeDefs.Find(nd => nd.Id == node.DefinitionId);
            if (nodeDef == null)
            {
                Logger.Warn($"No definition found for salvage node '{node.DefinitionId}'.");
                return;
            }

            var rng = new DeterministicRng(node.Id);
            int yield = rng.NextInt(nodeDef.MinYield, nodeDef.MaxYield + 1);

            Logger.Log($"Claimed salvage node '{node.DefinitionId}' in room '{room.DefinitionId}' — yield: {yield}");

            foreach (string resId in nodeDef.ResourceIds)
            {
                _inventory.AddItem(resId, yield);
                _events.Publish(new SalvageClaimedEvent
                {
                    NodeId = node.DefinitionId,
                    ResourceId = resId,
                    Amount = yield
                });
            }

            _inventory.PrintContents();
        }

        private void Mine()
        {
            var camPos = _camera.Position;
            var front = _camera.Front;
            var origin = new System.Numerics.Vector3(camPos.X, camPos.Y, camPos.Z);
            var dir = new System.Numerics.Vector3(front.X, front.Y, front.Z);

            var hit = _raycaster.Cast(origin, dir, _chunkManager, 10f);
            if (hit.HasValue)
            {
                var vp = hit.Value.VoxelPosition;
                byte oldType = _chunkManager.GetVoxel(vp.X, vp.Y, vp.Z);
                Logger.Log($"Mined voxel at {vp}");
                _chunkManager.SetVoxel(vp.X, vp.Y, vp.Z, 0);

                // Award the appropriate resource for the mined voxel type.
                // Clamp to valid indices; any type outside the table falls back to "stone".
                string resourceId = oldType < VoxelTypeToResource.Length
                    ? VoxelTypeToResource[oldType]
                    : VoxelTypeToResource[0];
                _inventory.AddItem(resourceId, 1);

                _events.Publish(new VoxelMinedEvent { Position = vp, OldType = oldType });

                RebuildDirtyMeshes();
            }
        }

        private void LoadGame()
        {
            var delta = _saveManager.Load();
            if (delta == null) { Logger.Log("No save found."); return; }
            Logger.Log($"Loaded save with seed {delta.Seed}");
            foreach (var kv in delta.ChunkEdits)
            {
                var parts = kv.Key.Split(',');
                int cx = int.Parse(parts[0]), cy = int.Parse(parts[1]), cz = int.Parse(parts[2]);
                var chunk = _chunkManager.GetOrCreateChunk(cx, cy, cz);
                foreach (var edit in kv.Value)
                    chunk.SetVoxel(edit.X, edit.Y, edit.Z, edit.NewType);
            }
            foreach (var state in delta.SalvageStates)
            {
                foreach (var room in _layout.Rooms)
                {
                    if (room.Id != state.RoomId) continue;
                    foreach (var node in room.SalvageNodes)
                    {
                        if (node.Id == state.NodeId) node.IsClaimed = state.IsClaimed;
                    }
                }
            }
            if (delta.InventoryItems != null)
                _inventory.SetAllItems(delta.InventoryItems);
            RebuildDirtyMeshes();
        }

        private void OnRender(double dt)
        {
            _metrics.BeginFrame();

            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            _shader.Use();

            var view = _camera.GetViewMatrix();
            var proj = _camera.GetProjectionMatrix((float)_window.ClientSize.X / _window.ClientSize.Y);

            _shader.SetUniform("uView", view);
            _shader.SetUniform("uProjection", proj);
            _shader.SetUniform("uLightDir", new Vector3(0.5f, 1f, 0.3f));

            foreach (var kv in _gpuMeshes)
            {
                var (cx, cy, cz) = kv.Key;
                var model = Matrix4.CreateTranslation(
                    cx * VoxelChunk.SIZE,
                    cy * VoxelChunk.SIZE,
                    cz * VoxelChunk.SIZE);
                _shader.SetUniform("uModel", model);
                kv.Value.Render();
                _metrics.RecordChunk(kv.Value.TriangleCount);
            }
        }
    }
}
