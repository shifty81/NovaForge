using System.Numerics;
using Xunit;
using NovaForge.Voxels.Chunk;
using NovaForge.Voxels.Raycast;

namespace NovaForge.Tests
{
    public class VoxelRaycasterTests
    {
        private static ChunkManager SingleVoxelWorld(int wx, int wy, int wz, byte type = 1)
        {
            var chunks = new ChunkManager();
            chunks.SetVoxel(wx, wy, wz, type);
            return chunks;
        }

        [Fact]
        public void Cast_HitsVoxel_WhenRayIsAlignedWithIt()
        {
            var chunks = SingleVoxelWorld(5, 0, 0);
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(0, 0.5f, 0.5f), new Vector3(1, 0, 0), chunks, 20f);

            Assert.True(hit.HasValue);
            Assert.Equal(5, hit.Value.VoxelPosition.X);
            Assert.Equal(0, hit.Value.VoxelPosition.Y);
            Assert.Equal(0, hit.Value.VoxelPosition.Z);
        }

        [Fact]
        public void Cast_ReturnsNull_WhenNoVoxelInPath()
        {
            var chunks = new ChunkManager(); // empty world
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(0, 0, 0), new Vector3(1, 0, 0), chunks, 20f);

            Assert.Null(hit);
        }

        [Fact]
        public void Cast_ReturnsNull_WhenVoxelBeyondMaxDistance()
        {
            var chunks = SingleVoxelWorld(100, 0, 0);
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(0, 0.5f, 0.5f), new Vector3(1, 0, 0), chunks, 10f);

            Assert.Null(hit);
        }

        [Fact]
        public void Cast_ReturnsNull_WhenDirectionIsZeroVector()
        {
            var chunks = SingleVoxelWorld(1, 0, 0);
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(0, 0.5f, 0.5f), Vector3.Zero, chunks, 20f);

            Assert.Null(hit);
        }

        [Fact]
        public void Cast_HitsCorrectVoxel_WhenMultipleVoxelsPresentAlongRay()
        {
            var chunks = new ChunkManager();
            chunks.SetVoxel(2, 0, 0, 1);
            chunks.SetVoxel(5, 0, 0, 2);
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(0, 0.5f, 0.5f), new Vector3(1, 0, 0), chunks, 20f);

            // Should hit the closer voxel first.
            Assert.True(hit.HasValue);
            Assert.Equal(2, hit.Value.VoxelPosition.X);
        }

        [Fact]
        public void Cast_ReturnsCorrectNormal_WhenHitFromNegativeXSide()
        {
            var chunks = SingleVoxelWorld(5, 0, 0);
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(0, 0.5f, 0.5f), new Vector3(1, 0, 0), chunks, 20f);

            Assert.True(hit.HasValue);
            // Ray travels in +X direction, so the face hit is the -X face (normal points -X).
            Assert.Equal(-1, hit.Value.Normal.X);
            Assert.Equal(0, hit.Value.Normal.Y);
            Assert.Equal(0, hit.Value.Normal.Z);
        }

        [Fact]
        public void Cast_HitsVoxel_WhenRayTravelsInNegativeDirection()
        {
            var chunks = SingleVoxelWorld(0, 0, 0);
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(5, 0.5f, 0.5f), new Vector3(-1, 0, 0), chunks, 20f);

            Assert.True(hit.HasValue);
            Assert.Equal(0, hit.Value.VoxelPosition.X);
        }

        [Fact]
        public void Cast_HitsVoxelType_WhenTypeIsSet()
        {
            var chunks = new ChunkManager();
            chunks.SetVoxel(3, 0, 0, 3); // ore type
            var caster = new VoxelRaycaster();

            var hit = caster.Cast(new Vector3(0, 0.5f, 0.5f), new Vector3(1, 0, 0), chunks, 20f);

            Assert.True(hit.HasValue);
            // VoxelPosition should be the ore voxel.
            Assert.Equal(3, hit.Value.VoxelPosition.X);
        }
    }
}
