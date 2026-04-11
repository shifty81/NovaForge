using System;
using System.Collections.Generic;

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
