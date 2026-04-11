using System;
using System.Collections.Generic;
using NovaForge.Data.Models;

namespace NovaForge.Game
{
    public class Inventory
    {
        private readonly Dictionary<string, int> _items = new Dictionary<string, int>();

        public void AddItem(string id, int count)
        {
            if (_items.ContainsKey(id)) _items[id] += count;
            else _items[id] = count;
        }

        public int GetCount(string id)
        {
            return _items.TryGetValue(id, out int val) ? val : 0;
        }

        /// <summary>
        /// Attempts to remove the given quantities from the inventory atomically.
        /// Returns true and deducts all items if every item has sufficient stock;
        /// returns false and leaves the inventory unchanged otherwise.
        /// </summary>
        public bool TryRemoveItems(Dictionary<string, int> items)
        {
            if (items == null) return true;
            foreach (var kv in items)
                if (GetCount(kv.Key) < kv.Value) return false;
            foreach (var kv in items)
                _items[kv.Key] -= kv.Value;
            return true;
        }

        /// <summary>
        /// Returns the sum of <c>count × baseValue</c> for all held items.
        /// Resources not found in <paramref name="resourceDefs"/> contribute 0 value.
        /// </summary>
        public int GetTotalValue(List<ResourceDefinition> resourceDefs)
        {
            int total = 0;
            foreach (var kv in _items)
            {
                var def = resourceDefs.Find(r => r.Id == kv.Key);
                if (def != null) total += kv.Value * def.BaseValue;
            }
            return total;
        }

        /// <summary>Returns a snapshot copy of all inventory entries.</summary>
        public Dictionary<string, int> GetAllItems()
        {
            return new Dictionary<string, int>(_items);
        }

        /// <summary>Replaces all inventory contents with <paramref name="items"/>.</summary>
        public void SetAllItems(Dictionary<string, int> items)
        {
            _items.Clear();
            if (items == null) return;
            foreach (var kv in items)
                _items[kv.Key] = kv.Value;
        }

        public void PrintContents()
        {
            Console.WriteLine("=== Inventory ===");
            foreach (var kv in _items)
                Console.WriteLine($"  {kv.Key}: {kv.Value}");
        }
    }
}
