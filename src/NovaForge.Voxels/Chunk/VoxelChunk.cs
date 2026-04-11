using System.Collections.Generic;

namespace NovaForge.Voxels.Chunk
{
    public class VoxelChunk
    {
        public const int SIZE = 32;
        private readonly byte[,,] _voxels = new byte[SIZE, SIZE, SIZE];
        private readonly List<VoxelEdit> _edits = new List<VoxelEdit>();

        /// <summary>
        /// True when at least one voxel has been changed since the last <see cref="ClearDirty"/> call.
        /// Used by the renderer to rebuild only modified chunks instead of every chunk each frame.
        /// </summary>
        public bool IsDirty { get; private set; }

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
            IsDirty = true;
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

        /// <summary>Clears the dirty flag without discarding edit history.</summary>
        public void ClearDirty()
        {
            IsDirty = false;
        }

        /// <summary>
        /// Discards all recorded edits and clears the dirty flag.
        /// Call after world-generation writes so that the save system only
        /// serialises player-driven changes.
        /// </summary>
        public void ResetEdits()
        {
            _edits.Clear();
            IsDirty = false;
        }
    }
}
