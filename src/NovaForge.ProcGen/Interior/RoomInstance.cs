using System.Collections.Generic;
using NovaForge.Core.Math;

namespace NovaForge.ProcGen.Interior
{
    public class RoomInstance
    {
        public int Id { get; set; }
        public string DefinitionId { get; set; }
        public Vector3i Position { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
        public List<int> ConnectedRoomIds { get; set; } = new List<int>();
        public List<SalvageNodeInstance> SalvageNodes { get; set; } = new List<SalvageNodeInstance>();
    }
}
