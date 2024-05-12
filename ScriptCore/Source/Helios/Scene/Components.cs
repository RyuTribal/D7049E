using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Numerics;
using System.Threading.Tasks;

namespace Helios
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}
	public class TransformComponent : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
				return rotation;
			}
			set
			{
				InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
				return scale;
			}
			set
			{
				InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
			}
		}
	}


	public class CameraComponent : Component
	{
		// Use this for 3:rd person
		public void RotateAroundEntity(Vector2 rotation, float speed, bool inverse_controls)
		{
			InternalCalls.Camera_RotateAroundEntity(Entity.ID, ref rotation, speed, inverse_controls);
		}

		// Use this for 1:st person
		public void Rotate(Vector2 rotation, float speed, bool inverse_controls)
		{
			InternalCalls.Camera_Rotate(Entity.ID, ref rotation, speed, inverse_controls);
		}

		public Vector3 GetForwardDirection()
		{
			Vector3 direction = new Vector3(1.0f);
			InternalCalls.Camera_GetForwardDirection(Entity.ID, ref direction);
			return direction;
		}

		public Vector3 GetRightDirection()
		{
			Vector3 direction = new Vector3(1.0f);
			InternalCalls.Camera_GetRightDirection(Entity.ID, ref direction);
			return direction;
		}
	}



	public class BoxColliderComponent : Component
	{
		public Vector3 LinearVelocity
		{
			get
			{
				InternalCalls.BoxCollider_GetLinearVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.BoxCollider_SetLinearVelocity(Entity.ID, ref value);
			}
		}

		public void AddLinearVelocity(Vector3 velocity)
		{
			InternalCalls.BoxCollider_AddLinearVelocity(Entity.ID, ref velocity);
		}

		public void AddAngularVelocity(Vector3 velocity)
		{
			InternalCalls.BoxCollider_AddAngularVelocity(Entity.ID, ref velocity);
		}

		public void AddImpulse(Vector3 impulse)
		{
			InternalCalls.BoxCollider_AddImpulse(Entity.ID, ref impulse);
		}

		public void AddAngularImpulse(Vector3 impulse)
		{
			InternalCalls.BoxCollider_AddAngularImpulse(Entity.ID, ref impulse);
		}

		public void AddLinearAndAngularImpulse(Vector3 linear_impulse, Vector3 angular_impulse)
		{
			InternalCalls.BoxCollider_AddLinearAngularImpulse(Entity.ID, ref linear_impulse, ref angular_impulse);
		}
	}


	public class GlobalSoundsComponent : Component
	{
		public void PlaySoundAtIndex(int index)
		{
			InternalCalls.Sounds_PlaySoundAtIndexGlobal(Entity.ID, index);
		}
	}

	public class LocalSoundsComponent : Component
	{
		public void PlaySoundAtIndex(int index)
		{
			InternalCalls.Sounds_PlaySoundAtIndexLocal(Entity.ID, index);
		}
	}


	public class SphereColliderComponent : Component
	{
		public Vector3 LinearVelocity
		{
			get
			{
				InternalCalls.SphereCollider_GetLinearVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.SphereCollider_SetLinearVelocity(Entity.ID, ref value);
			}
		}

		public void AddLinearVelocity(Vector3 velocity)
		{
			InternalCalls.SphereCollider_AddLinearVelocity(Entity.ID, ref velocity);
		}

		public void AddAngularVelocity(Vector3 velocity)
		{
			InternalCalls.SphereCollider_AddAngularVelocity(Entity.ID, ref velocity);
		}

		public void AddImpulse(Vector3 impulse)
		{
			InternalCalls.SphereCollider_AddImpulse(Entity.ID, ref impulse);
		}

		public void AddAngularImpulse(Vector3 impulse)
		{
			InternalCalls.SphereCollider_AddAngularImpulse(Entity.ID, ref impulse);
		}

		public void AddLinearAndAngularImpulse(Vector3 linear_impulse, Vector3 angular_impulse)
		{
			InternalCalls.SphereCollider_AddLinearAngularImpulse(Entity.ID, ref linear_impulse, ref angular_impulse);
		}
	}
}
