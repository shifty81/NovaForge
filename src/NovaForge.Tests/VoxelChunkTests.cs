using Xunit;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Tests
{
    public class VoxelChunkTests
    {
        [Fact]
        public void NewChunk_IsNotDirty()
        {
            var chunk = new VoxelChunk();
            Assert.False(chunk.IsDirty);
        }

        [Fact]
        public void SetVoxel_MarksDirty()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(0, 0, 0, 1);
            Assert.True(chunk.IsDirty);
        }

        [Fact]
        public void ClearDirty_ResetsFlag()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(0, 0, 0, 1);
            chunk.ClearDirty();
            Assert.False(chunk.IsDirty);
        }

        [Fact]
        public void ClearDirty_PreservesEdits()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(1, 2, 3, 1);
            chunk.ClearDirty();

            var edits = chunk.GetEdits();
            Assert.Single(edits);
        }

        [Fact]
        public void ResetEdits_ClearsEditsAndDirtyFlag()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(0, 0, 0, 1);
            chunk.SetVoxel(1, 1, 1, 2);
            chunk.ResetEdits();

            Assert.False(chunk.IsDirty);
            Assert.Empty(chunk.GetEdits());
        }

        [Fact]
        public void ResetEdits_DoesNotAffectVoxelData()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(5, 6, 7, 2);
            chunk.ResetEdits();

            Assert.Equal(2, chunk.GetVoxel(5, 6, 7));
        }

        [Fact]
        public void AfterResetEdits_SetVoxel_MarksDirtyAgain()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(0, 0, 0, 1);
            chunk.ResetEdits();
            Assert.False(chunk.IsDirty);

            chunk.SetVoxel(0, 0, 0, 2);
            Assert.True(chunk.IsDirty);
        }
    }
}
