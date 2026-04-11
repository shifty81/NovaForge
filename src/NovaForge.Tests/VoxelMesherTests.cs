using System.Collections.Generic;
using System.Linq;
using Xunit;
using NovaForge.Voxels.Chunk;
using NovaForge.Voxels.Meshing;

namespace NovaForge.Tests
{
    public class VoxelMesherTests
    {
        [Fact]
        public void EmptyChunk_ProducesEmptyMesh()
        {
            var chunk = new VoxelChunk();
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            Assert.Empty(mesh.Vertices);
            Assert.Empty(mesh.Indices);
        }

        [Fact]
        public void SingleSolidVoxel_ProducesSixFaces()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(1, 1, 1, 1);
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            // 6 faces × 4 vertices × 6 floats (pos+normal) = 144 floats
            Assert.Equal(144, mesh.Vertices.Length);
            // 6 faces × 6 indices
            Assert.Equal(36, mesh.Indices.Length);
        }

        [Fact]
        public void TwoAdjacentSolidVoxels_GreedyMergesSharedFace()
        {
            // Two voxels side by side along X. Their shared X-face should be culled,
            // and the outer X-faces should be merged into one quad each by greedy meshing,
            // giving 10 faces total instead of 12.
            var chunk = new VoxelChunk();
            chunk.SetVoxel(0, 0, 0, 1);
            chunk.SetVoxel(1, 0, 0, 1);
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            // 10 faces × 4 verts × 6 floats = 240
            // (the two +X outer faces would merge into one quad of width 2 — only 1 quad)
            // Actually: -X face of (0,0,0) and +X face of (1,0,0) merge into 0 faces (shared),
            // +X face of (1,0,0) → 1 face, -X face of (0,0,0) → 1 face,
            // top/bottom/front/back: each pair of adjacent faces merges into 1 wide quad.
            // Total: 2 X-axis outer faces (1 merged each) + 4 sides × 1 merged face = 6 faces? 
            // Wait: -X of voxel (0,0,0) at x=0 and +X of voxel (1,0,0) at x=2 are 1 face each.
            // Then top, bottom, front, back each see 2 adjacent voxels → 1 greedy-merged quad each.
            // So 6 faces total, just like a single voxel but with wider quads.
            // 6 × 4 × 6 = 144 floats — same count as 1 voxel due to greedy merging.
            Assert.Equal(144, mesh.Vertices.Length);
            Assert.Equal(36, mesh.Indices.Length);
        }

        [Fact]
        public void FullChunk_ProducesOnlyBoundaryFaces()
        {
            var chunk = new VoxelChunk();
            chunk.Fill(1);
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            // A fully solid chunk exposes only its outer surface: 6 faces, each a SIZE×SIZE quad.
            // Greedy meshing merges the entire face into one quad per face direction.
            // 6 quads × 4 verts × 6 floats = 144
            Assert.Equal(144, mesh.Vertices.Length);
            Assert.Equal(36, mesh.Indices.Length);
        }

        [Fact]
        public void Mesh_IndexCount_IsMultipleOfSix()
        {
            var chunk = new VoxelChunk();
            chunk.Fill(1);
            chunk.SetVoxel(5, 5, 5, 0);
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            // Every quad emits exactly 6 indices (2 triangles), so total must be divisible by 6.
            Assert.Equal(0, mesh.Indices.Length % 6);
        }

        [Fact]
        public void Mesh_VertexCount_IsMultipleOfFour()
        {
            var chunk = new VoxelChunk();
            chunk.Fill(1);
            chunk.SetVoxel(10, 10, 10, 0);
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            // Every quad emits exactly 4 vertices (6 floats each), so vertex-float count = 4 * 6 * quads.
            Assert.Equal(0, (mesh.Vertices.Length / 6) % 4);
        }
    }
}
