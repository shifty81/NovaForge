using System.Collections.Generic;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Voxels.Meshing
{
    public class VoxelMesher
    {
        private static readonly int[,] FaceNormals = {
            {  1,  0,  0 }, { -1,  0,  0 },
            {  0,  1,  0 }, {  0, -1,  0 },
            {  0,  0,  1 }, {  0,  0, -1 }
        };

        private static readonly float[,,] FaceVertices = {
            { {1,0,0},{1,1,0},{1,1,1},{1,0,1} },
            { {0,0,1},{0,1,1},{0,1,0},{0,0,0} },
            { {0,1,0},{0,1,1},{1,1,1},{1,1,0} },
            { {0,0,1},{0,0,0},{1,0,0},{1,0,1} },
            { {1,0,1},{1,1,1},{0,1,1},{0,0,1} },
            { {0,0,0},{0,1,0},{1,1,0},{1,0,0} }
        };

        public MeshData GenerateMesh(VoxelChunk chunk)
        {
            var vertices = new List<float>();
            var indices = new List<int>();
            int vertexOffset = 0;

            for (int x = 0; x < VoxelChunk.SIZE; x++)
            for (int y = 0; y < VoxelChunk.SIZE; y++)
            for (int z = 0; z < VoxelChunk.SIZE; z++)
            {
                if (chunk.IsAir(x, y, z)) continue;

                for (int f = 0; f < 6; f++)
                {
                    int nx = x + FaceNormals[f, 0];
                    int ny = y + FaceNormals[f, 1];
                    int nz = z + FaceNormals[f, 2];

                    bool neighborAir = nx < 0 || nx >= VoxelChunk.SIZE ||
                                       ny < 0 || ny >= VoxelChunk.SIZE ||
                                       nz < 0 || nz >= VoxelChunk.SIZE ||
                                       chunk.IsAir(nx, ny, nz);

                    if (!neighborAir) continue;

                    float fnx = FaceNormals[f, 0];
                    float fny = FaceNormals[f, 1];
                    float fnz = FaceNormals[f, 2];

                    for (int v = 0; v < 4; v++)
                    {
                        vertices.Add(x + FaceVertices[f, v, 0]);
                        vertices.Add(y + FaceVertices[f, v, 1]);
                        vertices.Add(z + FaceVertices[f, v, 2]);
                        vertices.Add(fnx);
                        vertices.Add(fny);
                        vertices.Add(fnz);
                    }

                    indices.Add(vertexOffset + 0);
                    indices.Add(vertexOffset + 1);
                    indices.Add(vertexOffset + 2);
                    indices.Add(vertexOffset + 0);
                    indices.Add(vertexOffset + 2);
                    indices.Add(vertexOffset + 3);
                    vertexOffset += 4;
                }
            }

            return new MeshData
            {
                Vertices = vertices.ToArray(),
                Indices = indices.ToArray()
            };
        }
    }
}
