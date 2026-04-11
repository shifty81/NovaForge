using System.Collections.Generic;

namespace NovaForge.Voxels.Chunk
{
    public class ChunkManager
    {
        private readonly Dictionary<(int, int, int), VoxelChunk> _chunks = new Dictionary<(int, int, int), VoxelChunk>();

        public VoxelChunk GetOrCreateChunk(int cx, int cy, int cz)
        {
            var key = (cx, cy, cz);
            if (!_chunks.TryGetValue(key, out var chunk))
            {
                chunk = new VoxelChunk();
                _chunks[key] = chunk;
            }
            return chunk;
        }

        private (int cx, int cy, int cz, int lx, int ly, int lz) WorldToLocal(int wx, int wy, int wz)
        {
            int cx = System.Math.DivRem(wx, VoxelChunk.SIZE, out int lx);
            int cy = System.Math.DivRem(wy, VoxelChunk.SIZE, out int ly);
            int cz = System.Math.DivRem(wz, VoxelChunk.SIZE, out int lz);
            if (lx < 0) { lx += VoxelChunk.SIZE; cx--; }
            if (ly < 0) { ly += VoxelChunk.SIZE; cy--; }
            if (lz < 0) { lz += VoxelChunk.SIZE; cz--; }
            return (cx, cy, cz, lx, ly, lz);
        }

        public void SetVoxel(int wx, int wy, int wz, byte type)
        {
            var (cx, cy, cz, lx, ly, lz) = WorldToLocal(wx, wy, wz);
            GetOrCreateChunk(cx, cy, cz).SetVoxel(lx, ly, lz, type);
        }

        public byte GetVoxel(int wx, int wy, int wz)
        {
            var (cx, cy, cz, lx, ly, lz) = WorldToLocal(wx, wy, wz);
            var key = (cx, cy, cz);
            if (!_chunks.TryGetValue(key, out var chunk)) return 0;
            return chunk.GetVoxel(lx, ly, lz);
        }

        public Dictionary<(int, int, int), VoxelChunk> GetChunks()
        {
            return new Dictionary<(int, int, int), VoxelChunk>(_chunks);
        }

        public Dictionary<(int, int, int), List<VoxelEdit>> GetAllEdits()
        {
            var result = new Dictionary<(int, int, int), List<VoxelEdit>>();
            foreach (var kv in _chunks)
                result[kv.Key] = kv.Value.GetEdits();
            return result;
        }
    }
}
