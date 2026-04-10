using System;
using OpenTK.Graphics.OpenGL4;
using OpenTK.Mathematics;

namespace NovaForge.Render.Shaders
{
    public class ShaderProgram : IDisposable
    {
        public int Handle { get; private set; }

        public static readonly string VertexSource = @"#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 vNormal;
void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
    vNormal = aNormal;
}";

        public static readonly string FragmentSource = @"#version 330 core
in vec3 vNormal;
out vec4 FragColor;
uniform vec3 uLightDir;
uniform vec3 uColor;
void main() {
    float diff = max(dot(normalize(vNormal), normalize(uLightDir)), 0.2);
    FragColor = vec4(uColor * diff, 1.0);
}";

        public ShaderProgram(string vertSrc, string fragSrc)
        {
            int vert = GL.CreateShader(ShaderType.VertexShader);
            GL.ShaderSource(vert, vertSrc);
            GL.CompileShader(vert);
            CheckShader(vert, "vertex");

            int frag = GL.CreateShader(ShaderType.FragmentShader);
            GL.ShaderSource(frag, fragSrc);
            GL.CompileShader(frag);
            CheckShader(frag, "fragment");

            Handle = GL.CreateProgram();
            GL.AttachShader(Handle, vert);
            GL.AttachShader(Handle, frag);
            GL.LinkProgram(Handle);
            GL.GetProgram(Handle, GetProgramParameterName.LinkStatus, out int status);
            if (status == 0)
                throw new Exception("Shader link error: " + GL.GetProgramInfoLog(Handle));

            GL.DetachShader(Handle, vert);
            GL.DetachShader(Handle, frag);
            GL.DeleteShader(vert);
            GL.DeleteShader(frag);
        }

        private static void CheckShader(int shader, string type)
        {
            GL.GetShader(shader, ShaderParameter.CompileStatus, out int status);
            if (status == 0)
                throw new Exception($"Shader ({type}) compile error: " + GL.GetShaderInfoLog(shader));
        }

        public void Use() => GL.UseProgram(Handle);

        public void SetUniform(string name, Matrix4 value)
        {
            int loc = GL.GetUniformLocation(Handle, name);
            GL.UniformMatrix4(loc, false, ref value);
        }

        public void SetUniform(string name, Vector3 value)
        {
            int loc = GL.GetUniformLocation(Handle, name);
            GL.Uniform3(loc, value);
        }

        public void SetUniform(string name, float value)
        {
            int loc = GL.GetUniformLocation(Handle, name);
            GL.Uniform1(loc, value);
        }

        public void SetUniform(string name, int value)
        {
            int loc = GL.GetUniformLocation(Handle, name);
            GL.Uniform1(loc, value);
        }

        public void Dispose()
        {
            GL.DeleteProgram(Handle);
        }
    }
}
