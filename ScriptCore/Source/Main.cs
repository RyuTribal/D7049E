using System;
using System.Runtime.CompilerServices;

namespace Helios
{
	public struct Vector3
	{
		public float X, Y, Z;

		public Vector3(float x, float y, float z)
		{
			X = x; 
			Y = y; 
			Z = z;
		}
	}
	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string text, int parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLogVector(ref Vector3 parameter, out Vector3 destination);
	}

	public class Entity
	{
		public Entity()
		{

		}
	}
	public class Main
	{
		public float FloatVar { get; set; }
		public Main()
		{
			Console.WriteLine("Constructor!");
			InternalCalls.NativeLog("Aint no way", 123);

			Vector3 pos = new Vector3(5, 1, 2);
			Vector3 res;
			InternalCalls.NativeLogVector(ref pos, out res);

			Console.WriteLine($"{res.X}, {res.Y}, {res.Z}");

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
