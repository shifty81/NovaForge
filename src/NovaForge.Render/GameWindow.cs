using System;
using OpenTK.Windowing.Desktop;
using OpenTK.Windowing.Common;
using OpenTK.Mathematics;

namespace NovaForge.Render
{
    public class NovaForgeWindow : GameWindow
    {
        public Action<double> UpdateCallback { get; set; }
        public Action<double> RenderCallback { get; set; }

        public NovaForgeWindow(int width, int height, string title)
            : base(
                new GameWindowSettings { UpdateFrequency = 60 },
                new NativeWindowSettings
                {
                    ClientSize = new Vector2i(width, height),
                    Title = title,
                    Flags = ContextFlags.ForwardCompatible
                })
        {
        }

        protected override void OnLoad()
        {
            base.OnLoad();
        }

        protected override void OnRenderFrame(FrameEventArgs args)
        {
            base.OnRenderFrame(args);
            RenderCallback?.Invoke(args.Time);
            SwapBuffers();
        }

        protected override void OnUpdateFrame(FrameEventArgs args)
        {
            base.OnUpdateFrame(args);
            UpdateCallback?.Invoke(args.Time);
        }

        protected override void OnResize(ResizeEventArgs args)
        {
            base.OnResize(args);
            OpenTK.Graphics.OpenGL4.GL.Viewport(0, 0, args.Width, args.Height);
        }
    }
}
