using NovaForge.Core.Math;

namespace NovaForge.Core.Events
{
    /// <summary>Published when the player mines a voxel.</summary>
    public struct VoxelMinedEvent
    {
        public Vector3i Position;
        public byte OldType;
    }

    /// <summary>Published when the player claims a salvage node.</summary>
    public struct SalvageClaimedEvent
    {
        public string NodeId;
        public string ResourceId;
        public int Amount;
    }
}
