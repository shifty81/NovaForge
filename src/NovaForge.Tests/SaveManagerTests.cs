using System;
using System.Collections.Generic;
using System.IO;
using Xunit;
using NovaForge.Data.Models;
using NovaForge.Game;
using NovaForge.Game.SaveLoad;
using NovaForge.ProcGen.Interior;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Tests
{
    public class SaveManagerTests : IDisposable
    {
        private readonly string _tmpPath;
        private readonly SaveManager _sut;

        public SaveManagerTests()
        {
            _tmpPath = Path.Combine(Path.GetTempPath(), $"novaforge_test_{Guid.NewGuid()}.json");
            _sut = new SaveManager(_tmpPath);
        }

        public void Dispose()
        {
            if (File.Exists(_tmpPath))
                File.Delete(_tmpPath);
        }

        private static (ChunkManager chunks, InteriorLayout layout) MakeEmpty()
        {
            var chunks = new ChunkManager();
            var layout = new InteriorLayout { Seed = 1L };
            return (chunks, layout);
        }

        [Fact]
        public void Load_ReturnsNull_WhenFileDoesNotExist()
        {
            var result = _sut.Load();
            Assert.Null(result);
        }

        [Fact]
        public void Save_CreatesFile()
        {
            var (chunks, layout) = MakeEmpty();
            _sut.Save(1L, chunks, layout);
            Assert.True(File.Exists(_tmpPath));
        }

        [Fact]
        public void SaveLoad_RoundTrips_Seed()
        {
            var (chunks, layout) = MakeEmpty();
            _sut.Save(99L, chunks, layout);
            var delta = _sut.Load();
            Assert.Equal(99L, delta.Seed);
        }

        [Fact]
        public void SaveLoad_RoundTrips_InventoryItems()
        {
            var (chunks, layout) = MakeEmpty();
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 7);
            inv.AddItem("rare_ore", 2);

            _sut.Save(1L, chunks, layout, inv);
            var delta = _sut.Load();

            Assert.NotNull(delta.InventoryItems);
            Assert.Equal(7, delta.InventoryItems["iron_scrap"]);
            Assert.Equal(2, delta.InventoryItems["rare_ore"]);
        }

        [Fact]
        public void SaveLoad_WithoutInventory_InventoryItemsIsEmpty()
        {
            var (chunks, layout) = MakeEmpty();
            _sut.Save(1L, chunks, layout);
            var delta = _sut.Load();
            // inventory not supplied → field initialised to empty dict by WorldDelta
            Assert.NotNull(delta.InventoryItems);
            Assert.Empty(delta.InventoryItems);
        }

        [Fact]
        public void SaveLoad_RoundTrips_VoxelEdits()
        {
            var (chunks, layout) = MakeEmpty();
            chunks.SetVoxel(1, 0, 1, 2);

            _sut.Save(1L, chunks, layout);
            var delta = _sut.Load();

            Assert.NotEmpty(delta.ChunkEdits);
        }
    }
}
