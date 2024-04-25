#pragma once

#include <glm/glm.hpp>

namespace Engine::Math {

	struct BoundingBox
	{
		glm::vec3 Min = glm::vec3(FLT_MAX);
		glm::vec3 Max = glm::vec3(-FLT_MAX);

		void ExpandBy(const glm::vec3& point)
		{
			Min = glm::min(Min, point);
			Max = glm::max(Max, point);
		}

		void ExpandBy(const BoundingBox& other)
		{
			ExpandBy(other.Min);
			ExpandBy(other.Max);
		}

		void TransformBy(const glm::mat4& matrix)
		{
			std::vector<glm::vec3> points = {
				{ Min.x, Min.y, Min.z },
				{ Max.x, Min.y, Min.z },
				{ Min.x, Max.y, Min.z },
				{ Max.x, Max.y, Min.z },
				{ Min.x, Min.y, Max.z },
				{ Max.x, Min.y, Max.z },
				{ Min.x, Max.y, Max.z },
				{ Max.x, Max.y, Max.z }
			};
			Min = glm::vec3(FLT_MAX);
			Max = glm::vec3(-FLT_MAX);

			for (const auto& point : points)
			{
				glm::vec4 transformedPoint = matrix * glm::vec4(point, 1.0f);
				ExpandBy(glm::vec3(transformedPoint));
			}
		}
	};

	struct Ray
	{
		glm::vec3 Origin, Direction;

		Ray(const glm::vec3& origin, const glm::vec3& direction)
		{
			Origin = origin;
			Direction = direction;
		}

		static Ray Zero()
		{
			return { {0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f} };
		}

		bool IntersectsAABB(const BoundingBox& aabb, float& t) const
		{
			glm::vec3 dirfrac;
			// r.dir is unit direction vector of ray
			dirfrac.x = 1.0f / Direction.x;
			dirfrac.y = 1.0f / Direction.y;
			dirfrac.z = 1.0f / Direction.z;
			// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
			// r.org is origin of ray
			const glm::vec3& lb = aabb.Min;
			const glm::vec3& rt = aabb.Max;
			float t1 = (lb.x - Origin.x) * dirfrac.x;
			float t2 = (rt.x - Origin.x) * dirfrac.x;
			float t3 = (lb.y - Origin.y) * dirfrac.y;
			float t4 = (rt.y - Origin.y) * dirfrac.y;
			float t5 = (lb.z - Origin.z) * dirfrac.z;
			float t6 = (rt.z - Origin.z) * dirfrac.z;

			float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
			float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

			// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
			if (tmax < 0)
			{
				t = tmax;
				return false;
			}

			// if tmin > tmax, ray doesn't intersect AABB
			if (tmin > tmax)
			{
				t = tmax;
				return false;
			}

			t = tmin;
			return true;
		}

		bool IntersectsTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, float& t) const
		{
			const glm::vec3 E1 = b - a;
			const glm::vec3 E2 = c - a;
			const glm::vec3 N = cross(E1, E2);
			const float det = -glm::dot(Direction, N);
			const float invdet = 1.f / det;
			const glm::vec3 AO = Origin - a;
			const glm::vec3 DAO = glm::cross(AO, Direction);
			const float u = glm::dot(E2, DAO) * invdet;
			const float v = -glm::dot(E1, DAO) * invdet;
			t = glm::dot(AO, N) * invdet;
			return (det >= 1e-6f && t >= 0.0f && u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f);
		}

	};



	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	float RandomFloat(float begin, float end);
	int RandomInt(int begin, int end);

}

