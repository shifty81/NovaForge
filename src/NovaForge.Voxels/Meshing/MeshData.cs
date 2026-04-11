namespace NovaForge.Voxels.Meshing
{
    public class MeshData
    {
        /// <summary>
        /// Number of floats per vertex in the interleaved vertex buffer:
        /// [x, y, z, nx, ny, nz, voxelType].
        /// </summary>
        public const int FloatsPerVertex = 7;

        public float[] Vertices { get; set; } = System.Array.Empty<float>();
        public int[] Indices { get; set; } = System.Array.Empty<int>();
    }
}
