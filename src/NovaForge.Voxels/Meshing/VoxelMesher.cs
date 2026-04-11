using System;
using System.Collections.Generic;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Voxels.Meshing
{
    /// <summary>
    /// Generates mesh data from a VoxelChunk using greedy meshing.
    /// Adjacent co-planar faces of the same voxel type are merged into larger quads,
    /// reducing triangle count compared to per-face emission.
    /// </summary>
    public class VoxelMesher
    {
        // Scratch masks reused per GenerateMesh call to avoid repeated allocations.
        private readonly byte[,] _maskPos = new byte[VoxelChunk.SIZE, VoxelChunk.SIZE];
        private readonly byte[,] _maskNeg = new byte[VoxelChunk.SIZE, VoxelChunk.SIZE];

        public MeshData GenerateMesh(VoxelChunk chunk)
        {
            var vertices = new List<float>();
            var indices = new List<int>();
            int vertexOffset = 0;

            // For each of the 3 axis directions
            for (int d = 0; d < 3; d++)
            {
                int u = (d + 1) % 3;
                int v = (d + 2) % 3;

                // Iterate over each "gap plane" from 0 to SIZE (inclusive).
                // The plane at position p lies between voxel[p-1] and voxel[p].
                for (int layer = 0; layer <= VoxelChunk.SIZE; layer++)
                {
                    Array.Clear(_maskPos, 0, _maskPos.Length);
                    Array.Clear(_maskNeg, 0, _maskNeg.Length);

                    for (int i = 0; i < VoxelChunk.SIZE; i++)
                    for (int j = 0; j < VoxelChunk.SIZE; j++)
                    {
                        byte left = 0, right = 0;
                        if (layer > 0)
                        {
                            left = GetVoxelByAxes(chunk, d, u, v, layer - 1, i, j);
                        }
                        if (layer < VoxelChunk.SIZE)
                        {
                            right = GetVoxelByAxes(chunk, d, u, v, layer, i, j);
                        }

                        // +d face: left solid, right air
                        if (left != 0 && right == 0) _maskPos[i, j] = left;
                        // -d face: left air, right solid
                        if (left == 0 && right != 0) _maskNeg[i, j] = right;
                    }

                    GreedyMerge(_maskPos, vertices, indices, ref vertexOffset, d, u, v, layer, +1);
                    GreedyMerge(_maskNeg, vertices, indices, ref vertexOffset, d, u, v, layer, -1);
                }
            }

            return new MeshData
            {
                Vertices = vertices.ToArray(),
                Indices = indices.ToArray()
            };
        }

        private static byte GetVoxelByAxes(VoxelChunk chunk, int d, int u, int v, int ld, int li, int lj)
        {
            int[] pos = new int[3];
            pos[d] = ld;
            pos[u] = li;
            pos[v] = lj;
            return chunk.GetVoxel(pos[0], pos[1], pos[2]);
        }

        private static void GreedyMerge(
            byte[,] mask,
            List<float> vertices,
            List<int> indices,
            ref int vertexOffset,
            int d, int u, int v,
            int layer,
            int normalSign)
        {
            float nx = d == 0 ? normalSign : 0f;
            float ny = d == 1 ? normalSign : 0f;
            float nz = d == 2 ? normalSign : 0f;

            for (int j = 0; j < VoxelChunk.SIZE; j++)
            for (int i = 0; i < VoxelChunk.SIZE; i++)
            {
                byte type = mask[i, j];
                if (type == 0) continue;

                // Expand width along u (i direction)
                int w = 1;
                while (i + w < VoxelChunk.SIZE && mask[i + w, j] == type)
                    w++;

                // Expand height along v (j direction)
                int h = 1;
                bool rowDone = false;
                while (!rowDone && j + h < VoxelChunk.SIZE)
                {
                    for (int k = i; k < i + w; k++)
                    {
                        if (mask[k, j + h] != type) { rowDone = true; break; }
                    }
                    if (!rowDone) h++;
                }

                // Clear merged region
                for (int dj = 0; dj < h; dj++)
                for (int di = 0; di < w; di++)
                    mask[i + di, j + dj] = 0;

                // Build quad corners: axes d, u, v → position array
                // c0=(i, j), c1=(i+w, j), c2=(i+w, j+h), c3=(i, j+h) in (u,v) space
                int[] c0 = new int[3]; c0[d] = layer; c0[u] = i;     c0[v] = j;
                int[] c1 = new int[3]; c1[d] = layer; c1[u] = i + w; c1[v] = j;
                int[] c2 = new int[3]; c2[d] = layer; c2[u] = i + w; c2[v] = j + h;
                int[] c3 = new int[3]; c3[d] = layer; c3[u] = i;     c3[v] = j + h;

                foreach (int[] c in new[] { c0, c1, c2, c3 })
                {
                    vertices.Add(c[0]); vertices.Add(c[1]); vertices.Add(c[2]);
                    vertices.Add(nx);   vertices.Add(ny);   vertices.Add(nz);
                    vertices.Add(type);
                }

                // CCW winding for +d faces; reversed for -d faces
                if (normalSign > 0)
                {
                    indices.Add(vertexOffset + 0); indices.Add(vertexOffset + 1); indices.Add(vertexOffset + 2);
                    indices.Add(vertexOffset + 0); indices.Add(vertexOffset + 2); indices.Add(vertexOffset + 3);
                }
                else
                {
                    indices.Add(vertexOffset + 0); indices.Add(vertexOffset + 3); indices.Add(vertexOffset + 2);
                    indices.Add(vertexOffset + 0); indices.Add(vertexOffset + 2); indices.Add(vertexOffset + 1);
                }

                vertexOffset += 4;
            }
        }
    }
}

