using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using Xunit;
using NovaForge.Data;
using NovaForge.Data.Models;

namespace NovaForge.Tests
{
    /// <summary>
    /// Tests for DataLoader. Each test writes minimal JSON to a temp directory and
    /// verifies that the loader deserialises it correctly.
    /// </summary>
    public class DataLoaderTests : IDisposable
    {
        private readonly string _dataPath;

        public DataLoaderTests()
        {
            _dataPath = Path.Combine(Path.GetTempPath(), $"novaforge_data_{Guid.NewGuid()}");
            Directory.CreateDirectory(_dataPath);
        }

        public void Dispose()
        {
            if (Directory.Exists(_dataPath))
                Directory.Delete(_dataPath, recursive: true);
        }

        // ── Helpers ──────────────────────────────────────────────────────────

        private string SubDir(string name)
        {
            var path = Path.Combine(_dataPath, name);
            Directory.CreateDirectory(path);
            return path;
        }

        private void WriteJson(string path, object obj)
        {
            File.WriteAllText(path, JsonSerializer.Serialize(obj));
        }

        // ── Resources ────────────────────────────────────────────────────────

        [Fact]
        public void LoadResources_ReturnsEmpty_WhenFileMissing()
        {
            var result = DataLoader.LoadResources(_dataPath);
            Assert.Empty(result);
        }

        [Fact]
        public void LoadResources_DeserialisesFields()
        {
            SubDir("resources");
            WriteJson(Path.Combine(_dataPath, "resources", "resources.json"), new[]
            {
                new { Id = "iron_scrap", Name = "Iron Scrap", Description = "desc", BaseValue = 5 }
            });

            var result = DataLoader.LoadResources(_dataPath);

            Assert.Single(result);
            Assert.Equal("iron_scrap", result[0].Id);
            Assert.Equal(5, result[0].BaseValue);
        }

        // ── Rooms ─────────────────────────────────────────────────────────────

        [Fact]
        public void LoadRooms_ReturnsEmpty_WhenDirectoryMissing()
        {
            var result = DataLoader.LoadRooms(_dataPath);
            Assert.Empty(result);
        }

        [Fact]
        public void LoadRooms_LoadsAllJsonFilesInDirectory()
        {
            var dir = SubDir("rooms");
            WriteJson(Path.Combine(dir, "entrance.json"), new
            {
                Id = "entrance", Archetype = "entrance",
                MinWidth = 5, MaxWidth = 8, MinHeight = 4, MaxHeight = 5,
                Tags = new[] { "spawn" },
                PossibleSalvageNodes = Array.Empty<string>()
            });
            WriteJson(Path.Combine(dir, "cargo.json"), new
            {
                Id = "cargo", Archetype = "cargo",
                MinWidth = 6, MaxWidth = 10, MinHeight = 4, MaxHeight = 6,
                Tags = new[] { "loot" },
                PossibleSalvageNodes = new[] { "salvage_ore" }
            });

            var result = DataLoader.LoadRooms(_dataPath);

            Assert.Equal(2, result.Count);
        }

        [Fact]
        public void LoadRooms_DeserialisesRoomFields()
        {
            var dir = SubDir("rooms");
            WriteJson(Path.Combine(dir, "room.json"), new
            {
                Id = "entrance", Archetype = "entrance",
                MinWidth = 5, MaxWidth = 8, MinHeight = 4, MaxHeight = 5,
                Tags = new[] { "spawn" },
                PossibleSalvageNodes = Array.Empty<string>()
            });

            var result = DataLoader.LoadRooms(_dataPath);

            Assert.Single(result);
            Assert.Equal("entrance", result[0].Archetype);
            Assert.Equal(8, result[0].MaxWidth);
        }

        // ── Salvage nodes ─────────────────────────────────────────────────────

        [Fact]
        public void LoadSalvageNodes_ReturnsEmpty_WhenFileMissing()
        {
            var result = DataLoader.LoadSalvageNodes(_dataPath);
            Assert.Empty(result);
        }

        [Fact]
        public void LoadSalvageNodes_DeserialisesFields()
        {
            SubDir("salvage");
            WriteJson(Path.Combine(_dataPath, "salvage", "salvage_nodes.json"), new[]
            {
                new { Id = "salvage_ore", Name = "Ore Vein", ResourceIds = new[] { "rare_ore" }, MinYield = 1, MaxYield = 3 }
            });

            var result = DataLoader.LoadSalvageNodes(_dataPath);

            Assert.Single(result);
            Assert.Equal("salvage_ore", result[0].Id);
            Assert.Equal(3, result[0].MaxYield);
        }

        // ── Crafting recipes ──────────────────────────────────────────────────

        [Fact]
        public void LoadRecipes_ReturnsEmpty_WhenFileMissing()
        {
            var result = DataLoader.LoadRecipes(_dataPath);
            Assert.Empty(result);
        }

        [Fact]
        public void LoadRecipes_DeserialisesFields()
        {
            SubDir("recipes");
            WriteJson(Path.Combine(_dataPath, "recipes", "recipes.json"), new[]
            {
                new
                {
                    Id = "iron_plate",
                    Name = "Iron Plate",
                    Ingredients = new Dictionary<string, int> { ["iron_scrap"] = 3 },
                    OutputId = "iron_plate",
                    OutputCount = 1
                }
            });

            var result = DataLoader.LoadRecipes(_dataPath);

            Assert.Single(result);
            Assert.Equal("iron_plate", result[0].Id);
            Assert.Equal(3, result[0].Ingredients["iron_scrap"]);
            Assert.Equal(1, result[0].OutputCount);
        }

        [Fact]
        public void LoadRecipes_DeserialisesMultipleIngredients()
        {
            SubDir("recipes");
            WriteJson(Path.Combine(_dataPath, "recipes", "recipes.json"), new[]
            {
                new
                {
                    Id = "alloy",
                    Name = "Alloy",
                    Ingredients = new Dictionary<string, int> { ["iron_scrap"] = 2, ["rare_ore"] = 1 },
                    OutputId = "alloy",
                    OutputCount = 2
                }
            });

            var result = DataLoader.LoadRecipes(_dataPath);

            Assert.Single(result);
            Assert.Equal(2, result[0].Ingredients.Count);
            Assert.Equal(2, result[0].OutputCount);
        }
    }
}
