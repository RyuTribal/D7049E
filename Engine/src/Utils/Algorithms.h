#pragma once

namespace Engine
{
	class Algorithms
	{
	public:
		// Uses Andrew's algorithm
		static void CreateConvexHull(Ref<std::vector<Ref<Vertex>>> hull, Ref<std::vector<Ref<Vertex>>> vertices, bool sort);

		static void TriangulateConvexMesh(Ref<std::vector<Ref<Vertex>>> vertices, std::vector<Ref<Triangle>>& triangles, std::vector<Ref<Edge>>* edge_list, bool sort);

		static float CalculateMaxDistance(const std::vector<Ref<Vertex>>& vertices, Vertex& p);

	private:
		// Helper functions
		inline static float Cross(const Vertex* origin, const Vertex* A, const Vertex* B)
		{
			return (A->coordinates.x - origin->coordinates.x) * (B->coordinates.y - origin->coordinates.y) - (A->coordinates.y - origin->coordinates.y) * (B->coordinates.x - origin->coordinates.x);
		}
	};
}