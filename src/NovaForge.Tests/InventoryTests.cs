using System.Collections.Generic;
using Xunit;
using NovaForge.Game;

namespace NovaForge.Tests
{
    public class InventoryTests
    {
        [Fact]
        public void GetCount_ReturnsZero_ForUnknownItem()
        {
            var inv = new Inventory();
            Assert.Equal(0, inv.GetCount("unknown"));
        }

        [Fact]
        public void AddItem_IncreasesCount()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 3);
            Assert.Equal(3, inv.GetCount("iron_scrap"));
        }

        [Fact]
        public void AddItem_AccumulatesAcrossCalls()
        {
            var inv = new Inventory();
            inv.AddItem("stone", 2);
            inv.AddItem("stone", 5);
            Assert.Equal(7, inv.GetCount("stone"));
        }

        [Fact]
        public void AddItem_IndependentItemsDoNotMix()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 10);
            inv.AddItem("rare_ore", 3);
            Assert.Equal(10, inv.GetCount("iron_scrap"));
            Assert.Equal(3, inv.GetCount("rare_ore"));
        }

        [Fact]
        public void GetAllItems_ReturnsSnapshot()
        {
            var inv = new Inventory();
            inv.AddItem("stone", 4);
            inv.AddItem("iron_scrap", 2);

            var all = inv.GetAllItems();

            Assert.Equal(2, all.Count);
            Assert.Equal(4, all["stone"]);
            Assert.Equal(2, all["iron_scrap"]);
        }

        [Fact]
        public void GetAllItems_IsSnapshot_NotLive()
        {
            var inv = new Inventory();
            inv.AddItem("stone", 1);
            var snapshot = inv.GetAllItems();

            inv.AddItem("stone", 10);

            // Snapshot should not reflect the subsequent change.
            Assert.Equal(1, snapshot["stone"]);
        }

        [Fact]
        public void SetAllItems_ReplacesContents()
        {
            var inv = new Inventory();
            inv.AddItem("stone", 100);

            inv.SetAllItems(new Dictionary<string, int> { ["rare_ore"] = 5 });

            Assert.Equal(0, inv.GetCount("stone"));
            Assert.Equal(5, inv.GetCount("rare_ore"));
        }

        [Fact]
        public void SetAllItems_WithNull_ClearsInventory()
        {
            var inv = new Inventory();
            inv.AddItem("stone", 10);

            inv.SetAllItems(null);

            Assert.Equal(0, inv.GetCount("stone"));
        }
    }
}
