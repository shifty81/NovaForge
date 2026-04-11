using System.Collections.Generic;
using NovaForge.ProcGen.Interior;
using NovaForge.Voxels.Chunk;

namespace NovaForge.ProcGen.Interior
{
    /// <summary>
    /// Stamps a procedurally generated InteriorLayout into a ChunkManager as voxel geometry.
    ///
    /// Each room grid cell is scaled by <see cref="Scale"/> voxels. Rooms receive a solid floor,
    /// solid walls, and an open top (no ceiling), making the interior easily navigable. Connected
    /// rooms have a doorway carved through the shared wall so the player can walk between them.
    /// </summary>
    public static class WorldBuilder
    {
        /// <summary>Voxels per room-grid unit.</summary>
        public const int Scale = 14;

        /// <summary>Height of room walls in voxels (floor at y=0, open above WallHeight).</summary>
        public const int WallHeight = 5;

        /// <summary>Width of doorways in voxels along the shared wall edge.</summary>
        private const int DoorWidth = 2;

        /// <summary>Height of doorways in voxels (y=1 to DoorHeight, inclusive).</summary>
        private const int DoorHeight = 3;

        /// <summary>Solid voxel type used for floors, walls, and ceilings.</summary>
        public const byte SolidType = 1;

        /// <summary>
        /// Stamps all rooms in <paramref name="layout"/> into <paramref name="chunks"/> as voxels,
        /// then carves doorways between every pair of directly connected rooms.
        /// </summary>
        public static void StampLayout(InteriorLayout layout, ChunkManager chunks)
        {
            // First pass: stamp each room's floor and walls.
            foreach (var room in layout.Rooms)
            {
                StampRoom(room, chunks);
            }

            // Second pass: carve doorways between connected room pairs (process each pair once).
            var carved = new HashSet<(int, int)>();
            foreach (var room in layout.Rooms)
            {
                foreach (int connId in room.ConnectedRoomIds)
                {
                    int lo = room.Id < connId ? room.Id : connId;
                    int hi = room.Id < connId ? connId : room.Id;
                    if (!carved.Add((lo, hi))) continue;

                    var other = layout.Rooms.Find(r => r.Id == connId);
                    if (other != null)
                        CarveDoorway(room, other, chunks);
                }
            }
        }

        private static void StampRoom(RoomInstance room, ChunkManager chunks)
        {
            int ox = room.Position.X * Scale;
            int oz = room.Position.Y * Scale; // grid Y maps to world Z

            // Room occupies world voxels [ox .. ox + room.Width + 1] × [oz .. oz + room.Height + 1]
            // (room.Width/Height are interior dimensions; +2 accounts for the two side walls)
            int totalW = room.Width + 2;
            int totalH = room.Height + 2;

            for (int dx = 0; dx < totalW; dx++)
            for (int dz = 0; dz < totalH; dz++)
            {
                int wx = ox + dx;
                int wz = oz + dz;

                bool isWall = dx == 0 || dx == totalW - 1 || dz == 0 || dz == totalH - 1;

                // Floor (y = 0)
                chunks.SetVoxel(wx, 0, wz, SolidType);

                // Walls (y = 1 .. WallHeight)
                if (isWall)
                {
                    for (int y = 1; y <= WallHeight; y++)
                        chunks.SetVoxel(wx, y, wz, SolidType);
                }
            }
        }

        private static void CarveDoorway(RoomInstance a, RoomInstance b, ChunkManager chunks)
        {
            int ax = a.Position.X, ay = a.Position.Y;
            int bx = b.Position.X, by = b.Position.Y;

            int ddx = bx - ax; // +1, -1, or 0
            int ddz = by - ay; // +1, -1, or 0

            if (ddx == 1)
            {
                // b is to the right (+X) of a
                int wallX = ax * Scale + a.Width + 1; // shared wall X
                int doorZ = ay * Scale + 1 + (a.Height / 2) - (DoorWidth / 2);
                CarveWallSegment(chunks, wallX, doorZ, isXWall: true);
            }
            else if (ddx == -1)
            {
                // b is to the left (-X) of a
                int wallX = bx * Scale + b.Width + 1;
                int doorZ = by * Scale + 1 + (b.Height / 2) - (DoorWidth / 2);
                CarveWallSegment(chunks, wallX, doorZ, isXWall: true);
            }
            else if (ddz == 1)
            {
                // b is behind (+Z) of a
                int wallZ = ay * Scale + a.Height + 1;
                int doorX = ax * Scale + 1 + (a.Width / 2) - (DoorWidth / 2);
                CarveWallSegment(chunks, doorX, wallZ, isXWall: false);
            }
            else if (ddz == -1)
            {
                // b is in front (-Z) of a
                int wallZ = by * Scale + b.Height + 1;
                int doorX = bx * Scale + 1 + (b.Width / 2) - (DoorWidth / 2);
                CarveWallSegment(chunks, doorX, wallZ, isXWall: false);
            }
        }

        /// <summary>
        /// Carves a door-sized opening. When <paramref name="isXWall"/> is true the wall runs
        /// perpendicular to the X axis (doorway spans Z); otherwise perpendicular to Z (spans X).
        /// </summary>
        private static void CarveWallSegment(ChunkManager chunks, int fixedCoord, int spanStart, bool isXWall)
        {
            for (int y = 1; y <= DoorHeight; y++)
            for (int s = 0; s < DoorWidth; s++)
            {
                if (isXWall)
                    chunks.SetVoxel(fixedCoord, y, spanStart + s, 0);
                else
                    chunks.SetVoxel(fixedCoord + s, y, spanStart, 0);
            }
        }
    }
}
