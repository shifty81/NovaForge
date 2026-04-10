using System;

namespace NovaForge.Core.Rng
{
    public class DeterministicRng
    {
        private Random _random;
        private long _seed;

        public long Seed => _seed;

        public DeterministicRng(long seed)
        {
            _seed = seed;
            _random = new Random((int)(seed ^ (seed >> 32)));
        }

        public int NextInt(int min, int max)
        {
            return _random.Next(min, max);
        }

        public float NextFloat()
        {
            return (float)_random.NextDouble();
        }

        public float NextFloat(float min, float max)
        {
            return min + (max - min) * NextFloat();
        }

        public bool NextBool()
        {
            return _random.Next(2) == 1;
        }

        public DeterministicRng Fork(string label)
        {
            long hash = _seed;
            foreach (char c in label)
            {
                hash = hash * 31 + c;
            }
            return new DeterministicRng(hash);
        }
    }
}
