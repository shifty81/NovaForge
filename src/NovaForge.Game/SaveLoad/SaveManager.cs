using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using NovaForge.ProcGen.Interior;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Game.SaveLoad
{
    public class SaveManager
    {
        public string SavePath { get; }

        public SaveManager(string savePath = "save.json")
        {
            SavePath = savePath;
        }

        private static readonly JsonSerializerOptions _options = new JsonSerializerOptions
        {
            WriteIndented = true,
            PropertyNameCaseInsensitive = true
        };

        public void Save(long seed, ChunkManager chunks, InteriorLayout layout, Inventory inventory = null)
        {
            var delta = new WorldDelta { Seed = seed };

            foreach (var kv in chunks.GetAllEdits())
            {
                string key = $"{kv.Key.Item1},{kv.Key.Item2},{kv.Key.Item3}";
                var edits = new List<VoxelEditDto>();
                foreach (var edit in kv.Value)
                    edits.Add(new VoxelEditDto(edit.X, edit.Y, edit.Z, edit.NewType));
                delta.ChunkEdits[key] = edits;
            }

            foreach (var room in layout.Rooms)
                foreach (var node in room.SalvageNodes)
                    delta.SalvageStates.Add(new SalvageNodeState(room.Id, node.Id, node.IsClaimed));

            if (inventory != null)
                delta.InventoryItems = inventory.GetAllItems();

            File.WriteAllText(SavePath, JsonSerializer.Serialize(delta, _options));
            Console.WriteLine($"[SaveManager] Saved to {SavePath}");
        }

        public WorldDelta Load()
        {
            if (!File.Exists(SavePath)) return null;
            var json = File.ReadAllText(SavePath);
            return JsonSerializer.Deserialize<WorldDelta>(json, _options);
        }
    }
}
