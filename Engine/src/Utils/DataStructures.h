#pragma once

/*
 * This file is for simplistic data structures
 */

namespace Engine
{

	struct Vertex
	{
        glm::vec3 coordinates = { 0.f, 0.f, 0.f };
        glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };
        glm::vec2 texture_coordinates = { 1.f, 1.f };
        glm::vec3 normal = { 0.0f, 0.0f, 0.0f };

        Vertex(glm::vec3 location, glm::vec4 color, glm::vec2 texture_coordinates, glm::vec3 normals) : coordinates(location), color(color), texture_coordinates(texture_coordinates), normal(normals) {}
		Vertex(glm::vec3 location, glm::vec4 color, glm::vec2 texture_coordinates) : coordinates(location), color(color), texture_coordinates(texture_coordinates) {}
        Vertex(glm::vec3 location, glm::vec4 color) : coordinates(location), color(color) {}
        Vertex(glm::vec3 location) : coordinates(location) {}
        Vertex() = default;

        void TransformData(std::vector<float>& target_vector) {
            target_vector.push_back(coordinates.x);
            target_vector.push_back(coordinates.y);
            target_vector.push_back(coordinates.z);

            target_vector.push_back(color.r);
            target_vector.push_back(color.g);
            target_vector.push_back(color.b);
            target_vector.push_back(color.a);

            target_vector.push_back(texture_coordinates.x);
            target_vector.push_back(texture_coordinates.y);

            target_vector.push_back(normal.x);
            target_vector.push_back(normal.y);
            target_vector.push_back(normal.z);
        }

        bool operator==(const Vertex& other) const {
            return std::abs(coordinates.x - other.coordinates.x) < EPSILON &&
                std::abs(coordinates.y - other.coordinates.y) < EPSILON &&
                std::abs(coordinates.z - other.coordinates.z) < EPSILON;
        }

        static float DistanceBetweenTwoVerts(const Vertex& v1, const Vertex v2) {
            return static_cast<float>(sqrt(pow(v2.coordinates.x - v1.coordinates.x, 2) + pow(v2.coordinates.y - v1.coordinates.y, 2) + pow(v2.coordinates.z - v1.coordinates.z, 2)));
        }
	};


    class Triangle {
    public:
        std::vector<Ref<Vertex>> vertices[3];
        std::vector<Ref<Triangle>> neighbors[3];

        Triangle(Ref<Vertex> v1, Ref<Vertex> v2,  Ref<Vertex> v3) {
            vertices->insert(vertices->begin() + 0, v1);
            vertices->insert(vertices->begin() + 1, v2);
            vertices->insert(vertices->begin() + 2, v3);
            neighbors->insert(neighbors->begin() + 0, nullptr);
            neighbors->insert(neighbors->begin() + 1, nullptr);
            neighbors->insert(neighbors->begin() + 2, nullptr);
        }

        int NeighborLength() {
            int length = 0;
            for (Ref<Triangle> neighbor : *neighbors) {
                if (neighbor != nullptr) {
                    length++;
                }
            }

            return length;
        }

        void SetNeighbor(int edgeIndex, Ref<Triangle> neighbor) {
            if (edgeIndex >= 0 && edgeIndex < 3) {
                neighbors->insert(neighbors->begin() + edgeIndex, neighbor);
            }
        }

        void SetColor(int r, int g, int b) 
        {
            for (Ref<Vertex>& vertex : *vertices) 
            {
                vertex->color = glm::vec4((float)r/255, (float)g/255, (float)b/255, 1.f);
            }
        }

        void SetColor(float r, float g, float b)
        {
            for (Ref<Vertex>& vertex : *vertices)
            {
                vertex->color = glm::vec4(r, g, b, 1.f);
            }
        }

        void SetOpacity(float alpha) 
        {
            for (Ref<Vertex>& vertex : *vertices)
            {
                vertex->color.a = alpha;
            }
        }

        int IndexOf(Ref<Triangle> neighbor) {
            for (int i = 0; i < 3; ++i) {
                if (neighbors->at(i) == neighbor) {
                    return i;
                }
            }
            return -1;
        }

        static float TriangleArea(Ref<Vertex> A, Ref<Vertex> B, Ref<Vertex> C) {
            return std::abs((A->coordinates.x * (B->coordinates.y - C->coordinates.y) + B->coordinates.x * (C->coordinates.y - A->coordinates.y) + C->coordinates.x * (A->coordinates.y - B->coordinates.y)) / 2.0f);
        }

        static bool IsPointInTriangle(Ref<Vertex> P, Ref<Triangle> triangle) {

            float fullArea = TriangleArea(triangle->vertices->at(0), triangle->vertices->at(1), triangle->vertices->at(2));

            float area1 = TriangleArea(P, triangle->vertices->at(1), triangle->vertices->at(2));
            float area2 = TriangleArea(triangle->vertices->at(0), P, triangle->vertices->at(2));
            float area3 = TriangleArea(triangle->vertices->at(0), triangle->vertices->at(1), P);


            return std::abs(fullArea - (area1 + area2 + area3)) < 0.0001f;
        }

        static int SharedEdge(Ref<Triangle> t1, Ref<Triangle> t2) {
            int sharedVertices = 0;
            int lastFoundIndex = -1;

            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    if (t1->vertices[i] == t2->vertices[j]) {
                        sharedVertices++;
                        lastFoundIndex = i;
                        break;
                    }
                }
            }

            if (sharedVertices == 2) {
                return lastFoundIndex;
            }
            return -1;
        }


        static void FlattenTriangles(const std::vector<Ref<Triangle>>& triangles, std::vector<Ref<Vertex>>& flattened_vertices)
        {
	        for(auto triangle : triangles)
	        {
                flattened_vertices.push_back(triangle->vertices->at(0));
                flattened_vertices.push_back(triangle->vertices->at(1));
                flattened_vertices.push_back(triangle->vertices->at(2));
	        }
        }


    };

    struct Edge {
        Ref<Vertex> v1, v2;
        Ref<Triangle> origin;

        Edge(Ref<Vertex> vertex1, Ref<Vertex> vertex2, Ref<Triangle> from_triangle) {
            v1 = vertex1;
            v2 = vertex2;
            origin = from_triangle;
        }

        bool operator==(const Edge& other) const {
            return (v1 == other.v1 && v2 == other.v2) ||
                (v1 == other.v2 && v2 == other.v1);
        }

        static int FindEdgeIndex(Ref<Edge> edge, Ref<Triangle> triangle) {
            for (int i = 0; i < 3; ++i) {
                Ref<Vertex> v1 = triangle->vertices->at(i);
                Ref<Vertex> v2 = triangle->vertices->at((i + 1) % 3);

                if ((*v1.get() == *edge->v1.get() && *v2.get() == *edge->v2.get()) ||
                    (*v1.get() == *edge->v2.get() && *v2.get() == *edge->v1.get())) {
                    return i;
                }
            }
            return -1;
        }
    };
}