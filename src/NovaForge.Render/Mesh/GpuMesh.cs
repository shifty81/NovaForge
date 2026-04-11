using System;
using OpenTK.Graphics.OpenGL4;
using NovaForge.Voxels.Meshing;

namespace NovaForge.Render.Mesh
{
    public class GpuMesh : IDisposable
    {
        private int _vao, _vbo, _ebo;
        private int _indexCount;

        public GpuMesh(MeshData data)
        {
            _vao = GL.GenVertexArray();
            _vbo = GL.GenBuffer();
            _ebo = GL.GenBuffer();
            Upload(data);
        }

        private void Upload(MeshData data)
        {
            GL.BindVertexArray(_vao);

            GL.BindBuffer(BufferTarget.ArrayBuffer, _vbo);
            GL.BufferData(BufferTarget.ArrayBuffer, data.Vertices.Length * sizeof(float), data.Vertices, BufferUsageHint.DynamicDraw);

            GL.BindBuffer(BufferTarget.ElementArrayBuffer, _ebo);
            GL.BufferData(BufferTarget.ElementArrayBuffer, data.Indices.Length * sizeof(int), data.Indices, BufferUsageHint.DynamicDraw);

            int stride = MeshData.FloatsPerVertex * sizeof(float);

            // aPosition (location 0): xyz
            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, stride, 0);
            GL.EnableVertexAttribArray(0);

            // aNormal (location 1): nx ny nz
            GL.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, stride, 3 * sizeof(float));
            GL.EnableVertexAttribArray(1);

            // aVoxelType (location 2): voxel material id
            GL.VertexAttribPointer(2, 1, VertexAttribPointerType.Float, false, stride, 6 * sizeof(float));
            GL.EnableVertexAttribArray(2);

            GL.BindVertexArray(0);

            _indexCount = data.Indices.Length;
        }

        public void Update(MeshData data)
        {
            GL.BindVertexArray(_vao);

            GL.BindBuffer(BufferTarget.ArrayBuffer, _vbo);
            GL.BufferData(BufferTarget.ArrayBuffer, data.Vertices.Length * sizeof(float), data.Vertices, BufferUsageHint.DynamicDraw);

            GL.BindBuffer(BufferTarget.ElementArrayBuffer, _ebo);
            GL.BufferData(BufferTarget.ElementArrayBuffer, data.Indices.Length * sizeof(int), data.Indices, BufferUsageHint.DynamicDraw);

            GL.BindVertexArray(0);
            _indexCount = data.Indices.Length;
        }

        /// <summary>Number of triangles in the last uploaded mesh (indexCount / 3).</summary>
        public int TriangleCount => _indexCount / 3;

        public void Render()
        {
            if (_indexCount == 0) return;
            GL.BindVertexArray(_vao);
            GL.DrawElements(PrimitiveType.Triangles, _indexCount, DrawElementsType.UnsignedInt, 0);
            GL.BindVertexArray(0);
        }

        public void Dispose()
        {
            GL.DeleteVertexArray(_vao);
            GL.DeleteBuffer(_vbo);
            GL.DeleteBuffer(_ebo);
        }
    }
}
