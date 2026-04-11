using System.Collections.Generic;
using Xunit;
using NovaForge.Data.Models;
using NovaForge.ProcGen.Interior;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Tests
{
    public class WorldBuilderTests
    {
        private static List<RoomDefinition> GetRooms() => new List<RoomDefinition>
        {
            new RoomDefinition { Id = "entrance", Archetype = "entrance", MinWidth = 4, MaxWidth = 4, MinHeight = 4, MaxHeight = 4, Tags = new List<string>(), PossibleSalvageNodes = new List<string>() },
            new RoomDefinition { Id = "corridor", Archetype = "corridor", MinWidth = 3, MaxWidth = 3, MinHeight = 3, MaxHeight = 3, Tags = new List<string>(), PossibleSalvageNodes = new List<string>() },
        };

        private static List<SalvageNodeDefinition> GetNodes() => new List<SalvageNodeDefinition>();

        [Fact]
        public void StampLayout_CreatesFloorVoxels()
        {
            var gen = new InteriorGenerator();
            var layout = gen.Generate(1L, GetRooms(), GetNodes());
            var chunks = new ChunkManager();

            WorldBuilder.StampLayout(layout, chunks);

            // The entrance room is at grid position (0,0). Interior floor voxel at world (1, 0, 1)
            // (offset 1 for the wall thickness) should be the floor type.
            Assert.Equal(WorldBuilder.FloorType, chunks.GetVoxel(1, 0, 1));
        }

        [Fact]
        public void StampLayout_InteriorIsAir()
        {
            var gen = new InteriorGenerator();
            var layout = gen.Generate(1L, GetRooms(), GetNodes());
            var chunks = new ChunkManager();

            WorldBuilder.StampLayout(layout, chunks);

            // The entrance room (grid 0,0) has Width=4, Height=4, walls at x=0 and x=5, z=0 and z=5.
            // Interior at (ox+1..ox+4, y=1..WallHeight-1, oz+1..oz+4) should be air.
            int ox = 0 * WorldBuilder.Scale;
            int oz = 0 * WorldBuilder.Scale;
            Assert.Equal(0, chunks.GetVoxel(ox + 2, 1, oz + 2));
            Assert.Equal(0, chunks.GetVoxel(ox + 2, WorldBuilder.WallHeight - 1, oz + 2));
        }

        [Fact]
        public void StampLayout_WallsAreWallType()
        {
            var gen = new InteriorGenerator();
            var layout = gen.Generate(1L, GetRooms(), GetNodes());
            var chunks = new ChunkManager();

            WorldBuilder.StampLayout(layout, chunks);

            // Left wall of entrance room at x = ox + 0, y=1..WallHeight should be WallType.
            int ox = 0 * WorldBuilder.Scale;
            int oz = 0 * WorldBuilder.Scale;
            Assert.Equal(WorldBuilder.WallType, chunks.GetVoxel(ox, 1, oz + 1));
            Assert.Equal(WorldBuilder.WallType, chunks.GetVoxel(ox, WorldBuilder.WallHeight, oz + 1));
        }

        [Fact]
        public void StampLayout_IsDeterministic()
        {
            var rooms = GetRooms();
            var nodes = GetNodes();
            var gen = new InteriorGenerator();
            var layout1 = gen.Generate(99L, rooms, nodes);
            var layout2 = gen.Generate(99L, rooms, nodes);

            var chunks1 = new ChunkManager();
            var chunks2 = new ChunkManager();
            WorldBuilder.StampLayout(layout1, chunks1);
            WorldBuilder.StampLayout(layout2, chunks2);

            // Same seed → identical voxel at fixed sample points
            Assert.Equal(chunks1.GetVoxel(0, 0, 0), chunks2.GetVoxel(0, 0, 0));
            Assert.Equal(chunks1.GetVoxel(1, 1, 1), chunks2.GetVoxel(1, 1, 1));
        }

        [Fact]
        public void StampLayout_SetsChunksDirty()
        {
            var gen = new InteriorGenerator();
            var layout = gen.Generate(1L, GetRooms(), GetNodes());
            var chunks = new ChunkManager();

            WorldBuilder.StampLayout(layout, chunks);

            // At least one chunk must be dirty after stamping.
            bool anyDirty = false;
            foreach (var kv in chunks.GetChunks())
                if (kv.Value.IsDirty) { anyDirty = true; break; }
            Assert.True(anyDirty);
        }

        [Fact]
        public void ResetAllEdits_ClearsEditsAndDirty()
        {
            var gen = new InteriorGenerator();
            var layout = gen.Generate(1L, GetRooms(), GetNodes());
            var chunks = new ChunkManager();

            WorldBuilder.StampLayout(layout, chunks);
            chunks.ResetAllEdits();

            // After reset, no chunks should be dirty and no edits should remain.
            foreach (var kv in chunks.GetChunks())
            {
                Assert.False(kv.Value.IsDirty);
                Assert.Empty(kv.Value.GetEdits());
            }
        }
    }
}
