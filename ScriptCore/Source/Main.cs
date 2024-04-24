using System;

namespace Engine
{
	public class Main
	{
		public float FloatVar { get; set; }
		public Main()
		{
			Console.WriteLine("Constructor!");
		}

		public void PrintMessage()
		{
			Console.WriteLine("Hello world from C#!");
		}

		public void PrintCustomMessage(string message)
		{
			Console.WriteLine($"C# says: {message}");
		}
	}
}
