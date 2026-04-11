using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using NovaForge.Data.Models;

namespace NovaForge.Data
{
    public static class DataLoader
    {
        private static readonly JsonSerializerOptions _options = new JsonSerializerOptions
        {
            PropertyNameCaseInsensitive = true
        };

        public static List<RoomDefinition> LoadRooms(string dataPath)
        {
            var result = new List<RoomDefinition>();
            var roomsDir = Path.Combine(dataPath, "rooms");
            if (!Directory.Exists(roomsDir)) return result;
            foreach (var file in Directory.GetFiles(roomsDir, "*.json"))
            {
                var json = File.ReadAllText(file);
                var room = JsonSerializer.Deserialize<RoomDefinition>(json, _options);
                if (room != null) result.Add(room);
            }
            return result;
        }

        public static List<ResourceDefinition> LoadResources(string dataPath)
        {
            var file = Path.Combine(dataPath, "resources", "resources.json");
            if (!File.Exists(file)) return new List<ResourceDefinition>();
            var json = File.ReadAllText(file);
            return JsonSerializer.Deserialize<List<ResourceDefinition>>(json, _options) ?? new List<ResourceDefinition>();
        }

        public static List<SalvageNodeDefinition> LoadSalvageNodes(string dataPath)
        {
            var file = Path.Combine(dataPath, "salvage", "salvage_nodes.json");
            if (!File.Exists(file)) return new List<SalvageNodeDefinition>();
            var json = File.ReadAllText(file);
            return JsonSerializer.Deserialize<List<SalvageNodeDefinition>>(json, _options) ?? new List<SalvageNodeDefinition>();
        }

        public static List<CraftingRecipe> LoadRecipes(string dataPath)
        {
            var file = Path.Combine(dataPath, "recipes", "recipes.json");
            if (!File.Exists(file)) return new List<CraftingRecipe>();
            var json = File.ReadAllText(file);
            return JsonSerializer.Deserialize<List<CraftingRecipe>>(json, _options) ?? new List<CraftingRecipe>();
        }
    }
}
