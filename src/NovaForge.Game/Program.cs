using System;

namespace NovaForge.Game
{
    class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            try
            {
                var game = new Game();
                game.Run();
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Fatal: {ex}");
                Environment.Exit(1);
            }
        }
    }
}
