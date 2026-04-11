using System;
using System.Collections.Generic;
using System.Linq;
using NovaForge.Core.Math;
using NovaForge.Core.Rng;
using NovaForge.Data.Models;

namespace NovaForge.ProcGen.Interior
{
    public class InteriorGenerator
    {
        public InteriorLayout Generate(long seed, List<RoomDefinition> roomDefs, List<SalvageNodeDefinition> nodeDefs)
        {
            if (roomDefs == null || roomDefs.Count == 0)
                throw new ArgumentException("roomDefs must not be empty. Ensure the data/rooms directory exists and contains valid JSON files.", nameof(roomDefs));

            var rng = new DeterministicRng(seed).Fork("Interior");
            var layout = new InteriorLayout { Seed = seed };

            int roomCount = rng.NextInt(5, 9);

            var entranceDef = roomDefs.FirstOrDefault(r => r.Archetype == "entrance") ?? roomDefs[0];
            var otherDefs = roomDefs.Where(r => r.Archetype != "entrance").ToList();

            var usedPositions = new HashSet<(int, int)>();

            var entrance = CreateRoom(0, entranceDef, new Vector3i(0, 0, 0), rng, nodeDefs);
            layout.Rooms.Add(entrance);
            layout.EntryRoomId = entrance.DefinitionId;
            usedPositions.Add((0, 0));

            int[] dx = { 1, 0, -1, 0 };
            int[] dy = { 0, 1, 0, -1 };

            for (int i = 1; i < roomCount; i++)
            {
                RoomDefinition def;
                if (otherDefs.Count > 0)
                    def = otherDefs[rng.NextInt(0, otherDefs.Count)];
                else
                    def = roomDefs[rng.NextInt(0, roomDefs.Count)];

                Vector3i pos = new Vector3i(i, 0, 0);
                bool placed = false;
                foreach (var existing in layout.Rooms)
                {
                    for (int d = 0; d < 4; d++)
                    {
                        int nx = existing.Position.X + dx[d];
                        int ny = existing.Position.Y + dy[d];
                        if (!usedPositions.Contains((nx, ny)))
                        {
                            pos = new Vector3i(nx, ny, 0);
                            placed = true;
                            break;
                        }
                    }
                    if (placed) break;
                }

                if (!placed)
                {
                    int fx = i;
                    int fy = i;
                    while (usedPositions.Contains((fx, fy))) { fx++; }
                    pos = new Vector3i(fx, fy, 0);
                }

                var room = CreateRoom(i, def, pos, rng, nodeDefs);
                usedPositions.Add((pos.X, pos.Y));

                foreach (var existing in layout.Rooms)
                {
                    int diffX = System.Math.Abs(existing.Position.X - room.Position.X);
                    int diffY = System.Math.Abs(existing.Position.Y - room.Position.Y);
                    if (diffX + diffY == 1)
                    {
                        room.ConnectedRoomIds.Add(existing.Id);
                        existing.ConnectedRoomIds.Add(room.Id);
                    }
                }

                layout.Rooms.Add(room);
            }

            return layout;
        }

        private RoomInstance CreateRoom(int id, RoomDefinition def, Vector3i pos, DeterministicRng rng, List<SalvageNodeDefinition> nodeDefs)
        {
            var room = new RoomInstance
            {
                Id = id,
                DefinitionId = def.Id,
                Position = pos,
                Width = rng.NextInt(def.MinWidth, def.MaxWidth + 1),
                Height = rng.NextInt(def.MinHeight, def.MaxHeight + 1)
            };

            if (def.PossibleSalvageNodes.Count > 0)
            {
                int nodeCount = rng.NextInt(1, 4);
                for (int n = 0; n < nodeCount; n++)
                {
                    string nodeDefId = def.PossibleSalvageNodes[rng.NextInt(0, def.PossibleSalvageNodes.Count)];
                    var nodeDef = nodeDefs.FirstOrDefault(nd => nd.Id == nodeDefId);
                    if (nodeDef == null) continue;

                    var node = new SalvageNodeInstance
                    {
                        Id = id * 100 + n,
                        DefinitionId = nodeDefId,
                        LocalPosition = new Vector3i(rng.NextInt(0, room.Width), 0, rng.NextInt(0, room.Height)),
                        IsClaimed = false
                    };
                    room.SalvageNodes.Add(node);
                }
            }

            return room;
        }
    }
}
