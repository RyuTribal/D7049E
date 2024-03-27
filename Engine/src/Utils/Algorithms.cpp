#include "pch.h"
#include "Algorithms.h"

namespace Engine
{
	void Algorithms::CreateConvexHull(Ref<std::vector<Ref<Vertex>>> hull, Ref<std::vector<Ref<Vertex>>> vertices, bool sort)
	{
		// Merge sort has the complexity O(n log n)

		CORE_ASSERT(vertices->size() >= 3, "Convex hull has to have at least 3 vertices!!");

		std::vector<Ref<Vertex>> upper_hull;
		std::vector<Ref<Vertex>> lower_hull;

		if (sort) {
			Sort::MergeSortArray(vertices->begin(), vertices->end(), [](Engine::Ref<Engine::Vertex> a, Engine::Ref<Engine::Vertex> b) {
				return a->coordinates.x == b->coordinates.x ? a->coordinates.y < b->coordinates.y : a->coordinates.x < b->coordinates.x;
				});
		}

		upper_hull.push_back(vertices->at(0));
		upper_hull.push_back(vertices->at(1));

		for(int i = 2; i < vertices->size(); i++)
		{
			upper_hull.push_back(vertices->at(i));
			while(upper_hull.size() > 2 && Cross(upper_hull[upper_hull.size() - 3].get(), upper_hull[upper_hull.size() - 2].get(), upper_hull[upper_hull.size() - 1].get()) <= 0)
			{
				upper_hull.erase(upper_hull.end() - 2);
			}
		}

		lower_hull.push_back(vertices->at(vertices->size() - 1));
		lower_hull.push_back(vertices->at(vertices->size() - 2));

		for (size_t i = vertices->size() - 3; i >= 0; i--)
		{
			lower_hull.push_back(vertices->at(i));
			while (lower_hull.size() > 2 && Cross(lower_hull[lower_hull.size() - 3].get(), lower_hull[lower_hull.size() - 2].get(), lower_hull[lower_hull.size() - 1].get()) <= 0)
			{
				lower_hull.erase(lower_hull.end() - 2);
			}
		}

		// So there are no duplicate vertices during combination of the lower and upper hull
		lower_hull.erase(lower_hull.begin());
		lower_hull.pop_back();
		for(auto vertex : upper_hull)
		{
			hull->push_back(vertex);
		}
		for (auto vertex : lower_hull)
		{
			hull->push_back(vertex);
		}
	}


	 void Algorithms::TriangulateConvexMesh(Ref<std::vector<Ref<Vertex>>> vertices, std::vector<Ref<Triangle>>& triangles, std::vector<Ref<Edge>>* edge_list, bool sort) {
		 CORE_ASSERT(vertices->size() >= 3, "Convex hull has to have at least 3 vertices!!");

		 std::vector<Ref<Vertex>> upper_hull;
		 std::vector<Ref<Vertex>> lower_hull;

		 if (sort) {
			 Sort::MergeSortArray(vertices->begin(), vertices->end(), [](Engine::Ref<Engine::Vertex> a, Engine::Ref<Engine::Vertex> b) {
				 return a->coordinates.x == b->coordinates.x ? a->coordinates.y < b->coordinates.y : a->coordinates.x < b->coordinates.x;
				 });
		 }

		 upper_hull.push_back(vertices->at(0));
		 upper_hull.push_back(vertices->at(1));

		 for (int i = 2; i < vertices->size(); i++)
		 {
			 upper_hull.push_back(vertices->at(i));
			 while (upper_hull.size() > 2 && Cross(upper_hull[upper_hull.size() - 3].get(), upper_hull[upper_hull.size() - 2].get(), upper_hull[upper_hull.size() - 1].get()) <= 0)
			 {
				 Ref<Vertex> v1_copy = CreateRef<Vertex>(*upper_hull.end()[-3].get());
				 Ref<Vertex> v2_copy = CreateRef<Vertex>(*upper_hull.end()[-2].get());
				 Ref<Vertex> v3_copy = CreateRef<Vertex>(*upper_hull.end()[-1].get());
				 triangles.push_back(CreateRef<Triangle>(v1_copy, v2_copy, v3_copy));
				 if (edge_list) {
					 edge_list->push_back(CreateRef<Edge>(upper_hull.end()[-3], upper_hull.end()[-2], triangles.at(triangles.size() - 1)));
					 edge_list->push_back(CreateRef<Edge>(upper_hull.end()[-3], upper_hull.end()[-1], triangles.at(triangles.size() - 1)));
					 edge_list->push_back(CreateRef<Edge>(upper_hull.end()[-2], upper_hull.end()[-1], triangles.at(triangles.size() - 1)));
				 }
				 upper_hull.erase(upper_hull.end() - 2);
			 }
		 }

		 lower_hull.push_back(vertices->at(vertices->size() - 1));
		 lower_hull.push_back(vertices->at(vertices->size() - 2));

		 for (size_t i = vertices->size() - 3; i >= 0; i--)
		 {
			 lower_hull.push_back(vertices->at(i));
			 while (lower_hull.size() > 2 && Cross(lower_hull[lower_hull.size() - 3].get(), lower_hull[lower_hull.size() - 2].get(), lower_hull[lower_hull.size() - 1].get()) <= 0)
			 {
				 Ref<Vertex> v1_copy = CreateRef<Vertex>(*lower_hull.end()[-3].get());
				 Ref<Vertex> v2_copy = CreateRef<Vertex>(*lower_hull.end()[-2].get());
				 Ref<Vertex> v3_copy = CreateRef<Vertex>(*lower_hull.end()[-1].get());
				 triangles.push_back(CreateRef<Triangle>(v1_copy, v2_copy, v3_copy));
				 if (edge_list) {
					 edge_list->push_back(CreateRef<Edge>(lower_hull.end()[-3], lower_hull.end()[-2], triangles.at(triangles.size() - 1)));
					 edge_list->push_back(CreateRef<Edge>(lower_hull.end()[-3], lower_hull.end()[-1], triangles.at(triangles.size() - 1)));
					 edge_list->push_back(CreateRef<Edge>(lower_hull.end()[-2], lower_hull.end()[-1], triangles.at(triangles.size() - 1)));
				 }
				 lower_hull.erase(lower_hull.end() - 2);
			 }
		 }
	}


	 float Algorithms::CalculateMaxDistance(const std::vector<Ref<Vertex>>& vertices, Vertex& p) {
		 float minX = std::numeric_limits<float>::max();
		 float maxX = std::numeric_limits<float>::min();
		 float minY = std::numeric_limits<float>::max();
		 float maxY = std::numeric_limits<float>::min();
		 float minZ = std::numeric_limits<float>::max();
		 float maxZ = std::numeric_limits<float>::min();

		 // Find the bounding box
		 for (const auto& vertex : vertices) {
			 minX = std::min(minX, vertex->coordinates.x);
			 maxX = std::max(maxX, vertex->coordinates.x);
			 minY = std::min(minY, vertex->coordinates.y);
			 maxY = std::max(maxY, vertex->coordinates.y);
			 minZ = std::min(minZ, vertex->coordinates.z);
			 maxZ = std::max(maxZ, vertex->coordinates.z);
		 }
		 p = Vertex({ (minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2 });

		 Vertex corner1({ minX, minY, minZ });
		 Vertex corner2({ maxX, maxY, maxZ });
		 float maxDistance = Vertex::DistanceBetweenTwoVerts(p, corner1);

		 maxDistance = std::max(maxDistance, Vertex::DistanceBetweenTwoVerts(p, corner2));

		 return maxDistance;
	 }

}