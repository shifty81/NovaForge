using NovaForge.Core.Math;

namespace NovaForge.ProcGen.Interior
{
    public class SalvageNodeInstance
    {
        public int Id { get; set; }
        public string DefinitionId { get; set; }
        public Vector3i LocalPosition { get; set; }
        public bool IsClaimed { get; set; }
    }
}
