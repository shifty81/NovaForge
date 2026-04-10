using System.Collections.Generic;

namespace NovaForge.ProcGen.Interior
{
    public class InteriorLayout
    {
        public long Seed { get; set; }
        public List<RoomInstance> Rooms { get; set; } = new List<RoomInstance>();
        public string EntryRoomId { get; set; }
    }
}
