using System;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Helios
{

	public class Player : Entity
	{
		private TransformComponent m_Transform;
		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
			m_Transform.Translation = new Vector3(1.0f);
			Console.WriteLine($"Player {ID} created");
			Console.WriteLine($"Has the transform component: {HasComponent<TransformComponent>()}");
		}

		void OnUpdate(float delta_time)
		{
			// Console.WriteLine($"Player updating, delta time {delta_time}");
			Vector3 vector3 = m_Transform.Translation;
			vector3.X += 1.0f * delta_time;
			m_Transform.Translation = vector3;
		}
	}
}
