using System.Collections.Generic;

namespace NovaForge.Data.Models
{
    public class RoomDefinition
    {
        public string Id { get; set; }
        public string Archetype { get; set; }
        public int MinWidth { get; set; }
        public int MaxWidth { get; set; }
        public int MinHeight { get; set; }
        public int MaxHeight { get; set; }
        public List<string> Tags { get; set; } = new List<string>();
        public List<string> PossibleSalvageNodes { get; set; } = new List<string>();
    }
}
