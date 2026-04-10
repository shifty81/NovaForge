using System.Collections.Generic;

namespace NovaForge.Data.Models
{
    public class SalvageNodeDefinition
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public List<string> ResourceIds { get; set; } = new List<string>();
        public int MinYield { get; set; }
        public int MaxYield { get; set; }
    }
}
