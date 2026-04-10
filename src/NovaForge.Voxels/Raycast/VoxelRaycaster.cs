using System;
using System.Numerics;
using NovaForge.Core.Math;
using NovaForge.Voxels.Chunk;

namespace NovaForge.Voxels.Raycast
{
    public class VoxelRaycaster
    {
        public struct RaycastHit
        {
            public Vector3 HitPoint;
            public Vector3i VoxelPosition;
            public Vector3i Normal;
        }

        public RaycastHit? Cast(Vector3 origin, Vector3 direction, ChunkManager chunks, float maxDistance)
        {
            if (direction.LengthSquared() < 1e-10f) return null;
            direction = Vector3.Normalize(direction);

            int x = (int)MathF.Floor(origin.X);
            int y = (int)MathF.Floor(origin.Y);
            int z = (int)MathF.Floor(origin.Z);

            int stepX = direction.X > 0 ? 1 : -1;
            int stepY = direction.Y > 0 ? 1 : -1;
            int stepZ = direction.Z > 0 ? 1 : -1;

            float tDeltaX = MathF.Abs(1f / direction.X);
            float tDeltaY = MathF.Abs(1f / direction.Y);
            float tDeltaZ = MathF.Abs(1f / direction.Z);

            float tMaxX = direction.X > 0
                ? ((x + 1) - origin.X) / direction.X
                : (origin.X - x) / -direction.X;
            float tMaxY = direction.Y > 0
                ? ((y + 1) - origin.Y) / direction.Y
                : (origin.Y - y) / -direction.Y;
            float tMaxZ = direction.Z > 0
                ? ((z + 1) - origin.Z) / direction.Z
                : (origin.Z - z) / -direction.Z;

            Vector3i normal = new Vector3i(0, 0, 0);
            float t = 0;

            while (t < maxDistance)
            {
                if (chunks.GetVoxel(x, y, z) != 0)
                {
                    return new RaycastHit
                    {
                        HitPoint = origin + direction * t,
                        VoxelPosition = new Vector3i(x, y, z),
                        Normal = normal
                    };
                }

                if (tMaxX < tMaxY && tMaxX < tMaxZ)
                {
                    t = tMaxX; tMaxX += tDeltaX;
                    x += stepX;
                    normal = new Vector3i(-stepX, 0, 0);
                }
                else if (tMaxY < tMaxZ)
                {
                    t = tMaxY; tMaxY += tDeltaY;
                    y += stepY;
                    normal = new Vector3i(0, -stepY, 0);
                }
                else
                {
                    t = tMaxZ; tMaxZ += tDeltaZ;
                    z += stepZ;
                    normal = new Vector3i(0, 0, -stepZ);
                }
            }

            return null;
        }
    }
}
