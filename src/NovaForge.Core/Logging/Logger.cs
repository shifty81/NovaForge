using System;

namespace NovaForge.Core.Logging
{
    public static class Logger
    {
        public static void Log(string message)
        {
            Console.WriteLine($"[{DateTime.Now:HH:mm:ss}] [INFO] {message}");
        }

        public static void Warn(string message)
        {
            Console.WriteLine($"[{DateTime.Now:HH:mm:ss}] [WARN] {message}");
        }

        public static void Error(string message)
        {
            Console.Error.WriteLine($"[{DateTime.Now:HH:mm:ss}] [ERROR] {message}");
        }
    }
}
