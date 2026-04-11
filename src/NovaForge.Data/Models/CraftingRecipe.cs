using System.Collections.Generic;

namespace NovaForge.Data.Models
{
    public class CraftingRecipe
    {
        public string Id { get; set; }
        public string Name { get; set; }
        /// <summary>Map of resource ID → quantity required to craft one output.</summary>
        public Dictionary<string, int> Ingredients { get; set; } = new Dictionary<string, int>();
        public string OutputId { get; set; }
        public int OutputCount { get; set; } = 1;
    }
}
