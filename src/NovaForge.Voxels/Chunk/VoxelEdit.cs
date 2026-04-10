namespace NovaForge.Voxels.Chunk
{
    public record VoxelEdit(int X, int Y, int Z, byte OldType, byte NewType);
}
