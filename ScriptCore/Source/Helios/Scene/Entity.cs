using System;
using System.Collections.Generic;
using System.Linq;
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
