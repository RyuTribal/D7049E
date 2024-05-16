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

		public Vector3 GetUpRightForwardDirection()
		{
			Vector3 direction = new Vector3(1.0f);
			InternalCalls.Camera_GetForwardDirection(Entity.ID, ref direction);
			direction.Y = 0;
			direction = Vector3.Normalize(direction);
			return direction;
		}

		public Vector3 GetUpRightRightDirection()
		{
			Vector3 direction = new Vector3(1.0f);
			InternalCalls.Camera_GetRightDirection(Entity.ID, ref direction);
			direction.Y = 0;
			direction = Vector3.Normalize(direction);
			return direction;
		}

		public Vector3 GetRightDirection()
		{
			Vector3 direction = new Vector3(1.0f);
			InternalCalls.Camera_GetRightDirection(Entity.ID, ref direction);
			return direction;
		}
		public Vector3 Position
		{
			get
			{
				InternalCalls.Camera_GetPosition(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.Camera_SetPosition(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.Camera_GetRotation(Entity.ID, out Vector3 rotation);
				return rotation;
			}
			set
			{
				InternalCalls.Camera_SetRotation(Entity.ID, ref value);
			}
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
		//
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

	public class CharacterControllerComponent : Component
	{
		public Vector3 LinearVelocity
		{
			get
			{
				InternalCalls.CharacterController_GetLinearVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.CharacterController_SetLinearVelocity(Entity.ID, ref value);
			}
		}

		public void AddLinearVelocity(Vector3 velocity)
		{
			InternalCalls.CharacterController_AddLinearVelocity(Entity.ID, ref velocity);
		}

		public void AddAngularVelocity(Vector3 velocity)
		{
			InternalCalls.CharacterController_AddAngularVelocity(Entity.ID, ref velocity);
		}

		public void AddImpulse(Vector3 impulse)
		{
			InternalCalls.CharacterController_AddImpulse(Entity.ID, ref impulse);
		}

		public void AddAngularImpulse(Vector3 impulse)
		{
			InternalCalls.CharacterController_AddAngularImpulse(Entity.ID, ref impulse);
		}

		public void AddLinearAndAngularImpulse(Vector3 linear_impulse, Vector3 angular_impulse)
		{
			InternalCalls.CharacterController_AddLinearAngularImpulse(Entity.ID, ref linear_impulse, ref angular_impulse);
		}

		public bool IsCharacterGrounded()
		{
			return InternalCalls.CharacterController_IsCharacterGrounded(Entity.ID);
		}

		public Vector3 GetRotation()
		{
			Vector3 rotation = new Vector3();
			InternalCalls.CharacterController_GetRotation(Entity.ID, ref rotation);
			return rotation;
		}

		public Quaternion GetRotationQuaternion()
		{
			Vector3 rotation = GetRotation();

			// Convert degrees to radians if necessary
			float yaw = rotation.Y * (float)Math.PI / 180.0f;
			float pitch = rotation.X * (float)Math.PI / 180.0f;
			float roll = rotation.Z * (float)Math.PI / 180.0f;

			// Calculate trigonometric functions
			float cy = (float)Math.Cos(yaw * 0.5f);
			float sy = (float)Math.Sin(yaw * 0.5f);
			float cp = (float)Math.Cos(pitch * 0.5f);
			float sp = (float)Math.Sin(pitch * 0.5f);
			float cr = (float)Math.Cos(roll * 0.5f);
			float sr = (float)Math.Sin(roll * 0.5f);

			// Create quaternion
			Quaternion q = new Quaternion();
			q.W = cr * cp * cy + sr * sp * sy;
			q.X = sr * cp * cy - cr * sp * sy;
			q.Y = cr * sp * cy + sr * cp * sy;
			q.Z = cr * cp * sy - sr * sp * cy;

			return q;

		}

		public void SetRotation(Vector3 rotation)
		{
			InternalCalls.CharacterController_SetRotation(Entity.ID, ref rotation);
		}

		public void Rotate(Vector3 rotation)
		{
			InternalCalls.CharacterController_Rotate(Entity.ID, ref rotation);
		}

		public void SetRotation(Quaternion rotation)
		{
			Vector3 euler;

			// Roll (X-axis rotation)
			double sinr_cosp = 2 * (rotation.W * rotation.X + rotation.Y * rotation.Z);
			double cosr_cosp = 1 - 2 * (rotation.X * rotation.X + rotation.Y * rotation.Y);
			euler.X = (float)Math.Atan2(sinr_cosp, cosr_cosp);

			// Pitch (Y-axis rotation)
			double sinp = 2 * (rotation.W * rotation.Y - rotation.Z * rotation.X);
			if (Math.Abs(sinp) >= 1)
				euler.Y = (float)Math.Abs(Math.PI / 2) * Math.Sign(sinp);
			else
				euler.Y = (float)Math.Asin(sinp);

			// Yaw (Z-axis rotation)
			double siny_cosp = 2 * (rotation.W * rotation.Z + rotation.X * rotation.Y);
			double cosy_cosp = 1 - 2 * (rotation.Y * rotation.Y + rotation.Z * rotation.Z);
			euler.Z = (float)Math.Atan2(siny_cosp, cosy_cosp);

			// Convert radians to degrees if your system uses degrees
			euler.X = euler.X * 180 / (float)Math.PI;
			euler.Y = euler.Y * 180 / (float)Math.PI;
			euler.Z = euler.Z * 180 / (float)Math.PI;

			SetRotation(euler);
		}
	}
}
