namespace NovaForge.Core.Math
{
    public static class MathHelpers
    {
        public static float Clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        public static int Clamp(int value, int min, int max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        public static float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        public static float ToRadians(float degrees)
        {
            return degrees * (System.MathF.PI / 180f);
        }

        public static float ToDegrees(float radians)
        {
            return radians * (180f / System.MathF.PI);
        }
    }
}
