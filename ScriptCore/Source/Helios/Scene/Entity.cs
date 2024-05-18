using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Helios
{
	public class Entity
	{
		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;


		public void SelfDestruct()
		{
			InternalCalls.Entity_Destroy(ID);
		}

		public void DestroyEntity(ulong id)
		{
			InternalCalls.Entity_Destroy(id);
		}

		public bool IsKeyPressed(KeyCode key_code)
		{
			return InternalCalls.IsKeyPressed(key_code);
		}

		public bool IsMouseButtonPressed(MouseButton button)
		{
			return InternalCalls.IsMouseButtonPressed(button);
		}

		public Vector2 GetMousePosition()
		{
			Vector2 position = new Vector2(1.0f);
			InternalCalls.GetMousePosition(ref position);
			return position;
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type component_type = typeof(T);
			return InternalCalls.Entity_HasComponent(ID, component_type);
		}
		public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
			{
				return null;
			}

			// Should probably cache this later. This is wasteful
			T component = new T() { Entity = this };
			return component;
		}
	}
}
