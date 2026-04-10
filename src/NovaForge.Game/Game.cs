using System;
using System.Collections.Generic;
using NovaForge.Core.Logging;
using NovaForge.Core.Math;
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
        private GpuMesh _gpuMesh;
        private ChunkManager _chunkManager;
        private InteriorLayout _layout;
        private Inventory _inventory;
        private SaveManager _saveManager;
        private VoxelMesher _mesher;
        private VoxelRaycaster _raycaster;
        private VoxelChunk _mainChunk;

        private long _seed = 42L;
        private bool _firstMouseMove = true;
        private float _lastMouseX, _lastMouseY;
        private const float MoveSpeed = 10f;

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

            _camera = new Camera { Position = new Vector3(16, 20, 40) };
            _shader = new ShaderProgram(ShaderProgram.VertexSource, ShaderProgram.FragmentSource);
            _mesher = new VoxelMesher();
            _raycaster = new VoxelRaycaster();
            _chunkManager = new ChunkManager();
            _inventory = new Inventory();
            _saveManager = new SaveManager();

            string dataPath = System.IO.Path.Combine(AppContext.BaseDirectory, "../../../../data");
            if (!System.IO.Directory.Exists(dataPath))
                dataPath = System.IO.Path.Combine(AppContext.BaseDirectory, "data");

            _roomDefs = DataLoader.LoadRooms(dataPath);
            _resourceDefs = DataLoader.LoadResources(dataPath);
            _nodeDefs = DataLoader.LoadSalvageNodes(dataPath);

            var generator = new InteriorGenerator();
            _layout = generator.Generate(_seed, _roomDefs, _nodeDefs);
            Logger.Log($"Generated {_layout.Rooms.Count} rooms.");

            _mainChunk = _chunkManager.GetOrCreateChunk(0, 0, 0);
            _mainChunk.Fill(1);
            for (int x = 0; x < VoxelChunk.SIZE; x++)
            for (int y = 16; y < VoxelChunk.SIZE; y++)
            for (int z = 0; z < VoxelChunk.SIZE; z++)
                _mainChunk.SetVoxel(x, y, z, 0);

            RebuildMesh();

            _window.CursorState = CursorState.Grabbed;
            Logger.Log("NovaForge loaded. WASD=move, mouse=look, E=scan, LMB=mine, F5=save, F9=load");
        }

        private void RebuildMesh()
        {
            var meshData = _mesher.GenerateMesh(_mainChunk);
            if (_gpuMesh == null)
                _gpuMesh = new GpuMesh(meshData);
            else
                _gpuMesh.Update(meshData);
        }

        private void OnUpdate(double dt)
        {
            var kb = _window.KeyboardState;
            var ms = _window.MouseState;
            float fdt = (float)dt;

            if (kb.IsKeyDown(Keys.Escape)) _window.Close();
            if (kb.IsKeyDown(Keys.W)) _camera.MoveForward(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.S)) _camera.MoveBack(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.A)) _camera.MoveLeft(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.D)) _camera.MoveRight(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.Space)) _camera.MoveUp(MoveSpeed, fdt);
            if (kb.IsKeyDown(Keys.LeftShift)) _camera.MoveDown(MoveSpeed, fdt);

            float mx = ms.X, my = ms.Y;
            if (_firstMouseMove) { _lastMouseX = mx; _lastMouseY = my; _firstMouseMove = false; }
            float dx = mx - _lastMouseX;
            float dy = my - _lastMouseY;
            _lastMouseX = mx; _lastMouseY = my;
            _camera.ProcessMouseMovement(dx, dy);

            if (kb.IsKeyPressed(Keys.E)) Scan();
            if (kb.IsKeyPressed(Keys.F5)) _saveManager.Save(_seed, _chunkManager, _layout);
            if (kb.IsKeyPressed(Keys.F9)) LoadGame();
            if (ms.IsButtonPressed(MouseButton.Left)) Mine();
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
                    float ddx = node.LocalPosition.X + room.Position.X * 10 - camPos.X;
                    float ddz = node.LocalPosition.Z + room.Position.Z * 10 - camPos.Z;
                    float dist = (float)System.Math.Sqrt(ddx * ddx + ddz * ddz);
                    if (dist <= scanRadius)
                    {
                        Logger.Log($"Found salvage node: {node.DefinitionId} in room {room.DefinitionId}. Claiming...");
                        node.IsClaimed = true;
                        _inventory.AddItem(node.DefinitionId, 1);
                        _inventory.PrintContents();
                        return;
                    }
                }
            }
            Logger.Log("No salvage nodes in range.");
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
                Logger.Log($"Mined voxel at {vp}");
                _chunkManager.SetVoxel(vp.X, vp.Y, vp.Z, 0);
                _inventory.AddItem("stone", 1);
                RebuildMesh();
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
            RebuildMesh();
        }

        private void OnRender(double dt)
        {
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            _shader.Use();

            var model = Matrix4.Identity;
            var view = _camera.GetViewMatrix();
            var proj = _camera.GetProjectionMatrix((float)_window.ClientSize.X / _window.ClientSize.Y);

            _shader.SetUniform("uModel", model);
            _shader.SetUniform("uView", view);
            _shader.SetUniform("uProjection", proj);
            _shader.SetUniform("uLightDir", new Vector3(0.5f, 1f, 0.3f));
            _shader.SetUniform("uColor", new Vector3(0.6f, 0.6f, 0.65f));

            _gpuMesh?.Render();
        }
    }
}
