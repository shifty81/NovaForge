using System.Collections.Generic;

namespace NovaForge.Game.SaveLoad
{
    public class WorldDelta
    {
        public long Seed { get; set; }
        public Dictionary<string, List<VoxelEditDto>> ChunkEdits { get; set; } = new Dictionary<string, List<VoxelEditDto>>();
        public List<SalvageNodeState> SalvageStates { get; set; } = new List<SalvageNodeState>();
    }
}
