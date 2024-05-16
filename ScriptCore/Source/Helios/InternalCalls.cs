using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Helios
{
	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsKeyPressed(KeyCode key);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool IsMouseButtonPressed(MouseButton button);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool GetMousePosition(ref Vector2 mouse_position_destination);


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entity_id, Type component_type);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_Destroy(ulong entity_id);


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_RotateAroundEntity(ulong entity_id, ref Vector2 rotation, float speed, bool inverse_controls);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_Rotate(ulong entity_id, ref Vector2 rotation, float speed, bool inverse_controls);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_GetForwardDirection(ulong entity_id,  ref Vector3 forward_direction);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_GetRightDirection(ulong entity_id, ref Vector3 right_direction);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_GetPosition(ulong entity_id, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_GetRotation(ulong entity_id, out Vector3 rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_SetPosition(ulong entity_id, ref Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Camera_SetRotation(ulong entity_id, ref Vector3 rotation);


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entity_id, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong entity_id, out Vector3 rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong entity_id, out Vector3 scale);


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entity_id, ref Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong entity_id, ref Vector3 rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong entity_id, ref Vector3 scale);



		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Sounds_PlaySoundAtIndexGlobal(ulong entity_id, int index);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Sounds_PlaySoundAtIndexLocal(ulong entity_id, int index);


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider_GetLinearVelocity(ulong entity_id, out Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider_SetLinearVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider_AddLinearVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider_AddAngularVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider_AddImpulse(ulong entity_id, ref Vector3 impulse);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider_AddAngularImpulse(ulong entity_id, ref Vector3 impulse);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider_AddLinearAngularImpulse(ulong entity_id, ref Vector3 linear_impulse, ref Vector3 angular_impulse);



		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SphereCollider_GetLinearVelocity(ulong entity_id, out Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SphereCollider_SetLinearVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SphereCollider_AddLinearVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SphereCollider_AddAngularVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SphereCollider_AddImpulse(ulong entity_id, ref Vector3 impulse);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SphereCollider_AddAngularImpulse(ulong entity_id, ref Vector3 impulse);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SphereCollider_AddLinearAngularImpulse(ulong entity_id, ref Vector3 linear_impulse, ref Vector3 angular_impulse);



		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CharacterController_GetLinearVelocity(ulong entity_id, out Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CharacterController_SetLinearVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CharacterController_AddLinearVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CharacterController_AddAngularVelocity(ulong entity_id, ref Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CharacterController_AddImpulse(ulong entity_id, ref Vector3 impulse);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CharacterController_AddAngularImpulse(ulong entity_id, ref Vector3 impulse);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CharacterController_AddLinearAngularImpulse(ulong entity_id, ref Vector3 linear_impulse, ref Vector3 angular_impulse);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool CharacterController_IsCharacterGrounded(ulong entity_id);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool CharacterController_GetRotation(ulong entity_id, ref Vector3 rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool CharacterController_SetRotation(ulong entity_id, ref Vector3 rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool CharacterController_Rotate(ulong entity_id, ref Vector3 rotation);

	}
}
