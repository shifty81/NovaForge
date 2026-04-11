using System;

namespace NovaForge.Core.Diagnostics
{
    /// <summary>
    /// Tracks per-frame performance counters: FPS, average frame time, and optional
    /// per-frame work counters (chunks rendered, triangles drawn).
    /// Call <see cref="BeginFrame"/> at the start of each frame and
    /// <see cref="EndFrame"/> at the end. <see cref="Sample"/> returns true once per
    /// second when a new measurement is available.
    /// </summary>
    public class PerformanceMetrics
    {
        private double _accumulator;
        private int _frameCount;

        // Current measurement interval values (updated once per second).
        private float _fps;
        private float _frameTimeMs;

        // Per-frame counters, reset each frame.
        private int _chunksRendered;
        private int _trianglesDrawn;

        // Snapshot values exposed after each Sample() == true.
        public float FPS => _fps;
        public float FrameTimeMs => _frameTimeMs;
        public int ChunksRendered => _chunksRendered;
        public int TrianglesDrawn => _trianglesDrawn;

        /// <summary>Reset per-frame counters. Call once at the start of each frame.</summary>
        public void BeginFrame()
        {
            _chunksRendered = 0;
            _trianglesDrawn = 0;
        }

        /// <summary>Record that <paramref name="dt"/> seconds elapsed this frame.</summary>
        public void EndFrame(double dt)
        {
            _accumulator += dt;
            _frameCount++;
        }

        /// <summary>Increment the chunks-rendered counter for the current frame.</summary>
        public void AddChunk(int triangles = 0)
        {
            _chunksRendered++;
            _trianglesDrawn += triangles;
        }

        /// <summary>
        /// Returns true (and updates <see cref="FPS"/> / <see cref="FrameTimeMs"/>) once
        /// per measurement interval (default 1 second). Call after <see cref="EndFrame"/>.
        /// </summary>
        public bool Sample(double intervalSeconds = 1.0)
        {
            if (_accumulator < intervalSeconds || _frameCount == 0)
                return false;

            _fps = (float)(_frameCount / _accumulator);
            _frameTimeMs = (float)(_accumulator / _frameCount * 1000.0);
            _accumulator = 0;
            _frameCount = 0;
            return true;
        }

        /// <summary>Returns a compact one-line summary suitable for a window title.</summary>
        public override string ToString() =>
            $"FPS: {_fps:F1}  Frame: {_frameTimeMs:F2} ms  Chunks: {_chunksRendered}  Tris: {_trianglesDrawn}";
    }
}
