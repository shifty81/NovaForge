using Xunit;
using NovaForge.Core.Diagnostics;

namespace NovaForge.Tests
{
    public class PerformanceMetricsTests
    {
        [Fact]
        public void Sample_ReturnsFalse_BeforeIntervalElapses()
        {
            var m = new PerformanceMetrics();
            m.EndFrame(0.5);
            Assert.False(m.Sample());
        }

        [Fact]
        public void Sample_ReturnsTrue_AfterOneSecond()
        {
            var m = new PerformanceMetrics();
            m.EndFrame(1.0);
            Assert.True(m.Sample());
        }

        [Fact]
        public void Sample_ResetsCounts_AfterInterval()
        {
            var m = new PerformanceMetrics();
            m.EndFrame(0.6);
            m.EndFrame(0.6); // 1.2 s total
            Assert.True(m.Sample());
            // After the sample the accumulator is reset; next partial second should return false.
            m.EndFrame(0.4);
            Assert.False(m.Sample());
        }

        [Fact]
        public void FPS_IsCorrect_ForTwoFramesInOneSecond()
        {
            var m = new PerformanceMetrics();
            m.EndFrame(0.5);
            m.EndFrame(0.5); // 2 frames in 1.0 s
            m.Sample();
            Assert.Equal(2f, m.FPS, 1);
        }

        [Fact]
        public void FrameTimeMs_IsCorrect_ForTwoFramesInOneSecond()
        {
            var m = new PerformanceMetrics();
            m.EndFrame(0.5);
            m.EndFrame(0.5);
            m.Sample();
            Assert.Equal(500f, m.FrameTimeMs, 0);
        }

        [Fact]
        public void BeginFrame_ResetsChunkAndTriangleCounters()
        {
            var m = new PerformanceMetrics();
            m.AddChunk(100);
            m.BeginFrame();
            Assert.Equal(0, m.ChunksRendered);
            Assert.Equal(0, m.TrianglesDrawn);
        }

        [Fact]
        public void AddChunk_AccumulatesCounters()
        {
            var m = new PerformanceMetrics();
            m.BeginFrame();
            m.AddChunk(300);
            m.AddChunk(150);
            Assert.Equal(2, m.ChunksRendered);
            Assert.Equal(450, m.TrianglesDrawn);
        }

        [Fact]
        public void ToString_ContainsFPSAndChunks()
        {
            var m = new PerformanceMetrics();
            m.BeginFrame();
            m.AddChunk(10);
            m.EndFrame(1.0);
            m.Sample();
            string s = m.ToString();
            Assert.Contains("FPS", s);
            Assert.Contains("Chunks", s);
        }

        [Fact]
        public void Sample_WithCustomInterval_ReturnsTrue_AfterInterval()
        {
            var m = new PerformanceMetrics();
            m.EndFrame(0.5);
            Assert.False(m.Sample(0.6));
            m.EndFrame(0.5); // 1.0 s total
            Assert.True(m.Sample(0.6));
        }
    }
}
