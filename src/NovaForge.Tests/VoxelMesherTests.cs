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

            // 6 faces × 4 vertices × FloatsPerVertex (pos+normal+type) floats
            Assert.Equal(6 * 4 * MeshData.FloatsPerVertex, mesh.Vertices.Length);
            // 6 faces × 6 indices
            Assert.Equal(36, mesh.Indices.Length);
        }

        [Fact]
        public void TwoAdjacentSolidVoxels_GreedyMergesSharedFace()
        {
            // Two voxels side by side along X. Their shared X-faces are culled.
            // Greedy meshing merges all remaining exposed faces into 6 wide quads (same count as
            // a single voxel, but the top/bottom/front/back quads are twice as wide).
            var chunk = new VoxelChunk();
            chunk.SetVoxel(0, 0, 0, 1);
            chunk.SetVoxel(1, 0, 0, 1);
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            // 6 greedy-merged quads × 4 verts × FloatsPerVertex floats
            Assert.Equal(6 * 4 * MeshData.FloatsPerVertex, mesh.Vertices.Length);
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
            // 6 quads × 4 verts × FloatsPerVertex floats
            Assert.Equal(6 * 4 * MeshData.FloatsPerVertex, mesh.Vertices.Length);
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

            // Every quad emits 4 vertices, each with FloatsPerVertex floats.
            Assert.Equal(0, mesh.Vertices.Length % (4 * MeshData.FloatsPerVertex));
        }

        [Fact]
        public void VoxelType_IsEncodedInSeventhFloat()
        {
            var chunk = new VoxelChunk();
            chunk.SetVoxel(0, 0, 0, 3); // ore type
            var mesher = new VoxelMesher();

            var mesh = mesher.GenerateMesh(chunk);

            // The 7th float of the first vertex (index 6) should equal the voxel type (3).
            Assert.Equal(3f, mesh.Vertices[6]);
        }
    }
}
