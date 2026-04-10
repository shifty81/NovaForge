using System;

namespace NovaForge.Core.Math
{
    public struct Vector3i : IEquatable<Vector3i>
    {
        public int X;
        public int Y;
        public int Z;

        public Vector3i(int x, int y, int z)
        {
            X = x; Y = y; Z = z;
        }

        public static Vector3i Zero => new Vector3i(0, 0, 0);

        public static Vector3i operator +(Vector3i a, Vector3i b) => new Vector3i(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        public static Vector3i operator -(Vector3i a, Vector3i b) => new Vector3i(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        public static Vector3i operator *(Vector3i a, int s) => new Vector3i(a.X * s, a.Y * s, a.Z * s);
        public static bool operator ==(Vector3i a, Vector3i b) => a.X == b.X && a.Y == b.Y && a.Z == b.Z;
        public static bool operator !=(Vector3i a, Vector3i b) => !(a == b);

        public bool Equals(Vector3i other) => this == other;
        public override bool Equals(object obj) => obj is Vector3i v && this == v;
        public override int GetHashCode() => HashCode.Combine(X, Y, Z);
        public override string ToString() => $"({X}, {Y}, {Z})";
    }
}
