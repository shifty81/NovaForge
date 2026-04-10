using System.Collections.Generic;

namespace NovaForge.Voxels.Chunk
{
    public class VoxelChunk
    {
        public const int SIZE = 32;
        private readonly byte[,,] _voxels = new byte[SIZE, SIZE, SIZE];
        private readonly List<VoxelEdit> _edits = new List<VoxelEdit>();

        public void Fill(byte type)
        {
            for (int x = 0; x < SIZE; x++)
                for (int y = 0; y < SIZE; y++)
                    for (int z = 0; z < SIZE; z++)
                        _voxels[x, y, z] = type;
        }

        public void SetVoxel(int x, int y, int z, byte type)
        {
            byte old = _voxels[x, y, z];
            RecordEdit(x, y, z, old, type);
            _voxels[x, y, z] = type;
        }

        public byte GetVoxel(int x, int y, int z)
        {
            return _voxels[x, y, z];
        }

        public bool IsAir(int x, int y, int z)
        {
            return _voxels[x, y, z] == 0;
        }

        public void RecordEdit(int x, int y, int z, byte oldType, byte newType)
        {
            _edits.Add(new VoxelEdit(x, y, z, oldType, newType));
        }

        public List<VoxelEdit> GetEdits()
        {
            return new List<VoxelEdit>(_edits);
        }
    }
}
