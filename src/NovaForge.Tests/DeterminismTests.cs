using System.Collections.Generic;
using System.Linq;
using Xunit;
using NovaForge.Data.Models;
using NovaForge.ProcGen.Interior;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Tests
{
    public class DeterminismTests
    {
        private static List<RoomDefinition> GetTestRooms()
        {
            return new List<RoomDefinition>
            {
                new RoomDefinition { Id = "entrance", Archetype = "entrance", MinWidth = 5, MaxWidth = 8, MinHeight = 4, MaxHeight = 5, Tags = new List<string>{"spawn"}, PossibleSalvageNodes = new List<string>() },
                new RoomDefinition { Id = "corridor", Archetype = "corridor", MinWidth = 3, MaxWidth = 5, MinHeight = 3, MaxHeight = 3, Tags = new List<string>{"traversal"}, PossibleSalvageNodes = new List<string>{"salvage_scrap"} },
                new RoomDefinition { Id = "cargo", Archetype = "cargo", MinWidth = 6, MaxWidth = 10, MinHeight = 4, MaxHeight = 6, Tags = new List<string>{"loot"}, PossibleSalvageNodes = new List<string>{"salvage_ore"} }
            };
        }

        private static List<SalvageNodeDefinition> GetTestNodes()
        {
            return new List<SalvageNodeDefinition>
            {
                new SalvageNodeDefinition { Id = "salvage_scrap", Name = "Scrap", ResourceIds = new List<string>{"iron_scrap"}, MinYield = 1, MaxYield = 5 },
                new SalvageNodeDefinition { Id = "salvage_ore", Name = "Ore Vein", ResourceIds = new List<string>{"rare_ore"}, MinYield = 1, MaxYield = 3 }
            };
        }

        [Fact]
        public void SameSeed_ProducesSameLayout()
        {
            var gen = new InteriorGenerator();
            var rooms = GetTestRooms();
            var nodes = GetTestNodes();

            var layout1 = gen.Generate(12345L, rooms, nodes);
            var layout2 = gen.Generate(12345L, rooms, nodes);

            Assert.Equal(layout1.Rooms.Count, layout2.Rooms.Count);
            for (int i = 0; i < layout1.Rooms.Count; i++)
            {
                Assert.Equal(layout1.Rooms[i].Position, layout2.Rooms[i].Position);
                Assert.Equal(layout1.Rooms[i].DefinitionId, layout2.Rooms[i].DefinitionId);
            }
        }

        [Fact]
        public void DifferentSeeds_ProduceDifferentLayouts()
        {
            var gen = new InteriorGenerator();
            var rooms = GetTestRooms();
            var nodes = GetTestNodes();

            var layout1 = gen.Generate(111L, rooms, nodes);
            var layout2 = gen.Generate(999L, rooms, nodes);

            bool differ = layout1.Rooms.Count != layout2.Rooms.Count;
            if (!differ)
            {
                for (int i = 0; i < layout1.Rooms.Count; i++)
                {
                    if (layout1.Rooms[i].Position != layout2.Rooms[i].Position ||
                        layout1.Rooms[i].DefinitionId != layout2.Rooms[i].DefinitionId)
                    {
                        differ = true;
                        break;
                    }
                }
            }
            Assert.True(differ);
        }

        [Fact]
        public void ApplyDeltas_RestoresState()
        {
            var original = new VoxelChunk();
            original.Fill(1);
            original.SetVoxel(1, 1, 1, 0);
            original.SetVoxel(5, 5, 5, 0);

            var edits = original.GetEdits();

            var restored = new VoxelChunk();
            restored.Fill(1);
            foreach (var edit in edits)
            {
                if (edit.NewType != edit.OldType)
                    restored.SetVoxel(edit.X, edit.Y, edit.Z, edit.NewType);
            }

            Assert.Equal(original.GetVoxel(1, 1, 1), restored.GetVoxel(1, 1, 1));
            Assert.Equal(original.GetVoxel(5, 5, 5), restored.GetVoxel(5, 5, 5));
            Assert.Equal(original.GetVoxel(0, 0, 0), restored.GetVoxel(0, 0, 0));
        }
    }
}
