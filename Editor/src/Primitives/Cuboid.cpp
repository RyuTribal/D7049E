#include "pch.h"
#include "Cuboid.h"

namespace Editor {
	Cuboid::Cuboid(float height, float width, float depth, Engine::UUID entity_id) : h(height), w(width), d(depth)
	{
        float half_width = w / 2.0f;
        float half_height = h / 2.0f;
        float half_depth = d / 2.0f;

        // Corner vertices
        std::vector<Engine::Vertex> vertices = {
            // Front face
            Engine::Vertex({-half_width, -half_height, half_depth}),
            Engine::Vertex({half_width, -half_height, half_depth}),
            Engine::Vertex({half_width, half_height, half_depth}),
            Engine::Vertex({-half_width, half_height, half_depth}),

            // Back face
            Engine::Vertex({-half_width, -half_height, -half_depth}),
            Engine::Vertex({half_width, -half_height, -half_depth}),
            Engine::Vertex({half_width, half_height, -half_depth}),
            Engine::Vertex({-half_width, half_height, -half_depth}),
        };
        
        // I add a center point to each face for beteer normals, just looks better shaded
        std::vector<glm::vec3> faceCenters = {
            (vertices[0].coordinates + vertices[1].coordinates + vertices[2].coordinates + vertices[3].coordinates) / 4.0f, // Front
            (vertices[1].coordinates + vertices[5].coordinates + vertices[6].coordinates + vertices[2].coordinates) / 4.0f, // Right
            (vertices[5].coordinates + vertices[4].coordinates + vertices[7].coordinates + vertices[6].coordinates) / 4.0f, // Back
            (vertices[4].coordinates + vertices[0].coordinates + vertices[3].coordinates + vertices[7].coordinates) / 4.0f, // Left
            (vertices[3].coordinates + vertices[2].coordinates + vertices[6].coordinates + vertices[7].coordinates) / 4.0f, // Top
            (vertices[4].coordinates + vertices[5].coordinates + vertices[1].coordinates + vertices[0].coordinates) / 4.0f  // Bottom
        };                                                                   

        std::vector<glm::vec3> normals = {
            glm::vec3(0.f, 0.f, 1.f),  // Front
            glm::vec3(1.f, 0.f, 0.f),  // Right
            glm::vec3(0.f, 0.f, -1.f), // Back
            glm::vec3(-1.f, 0.f, 0.f), // Left
            glm::vec3(0.f, 1.f, 0.f),  // Top
            glm::vec3(0.f, -1.f, 0.f)  // Bottom
        };

        for (int i = 0; i < 6; i++) {
            vertices.push_back(Engine::Vertex(faceCenters[i]));
            vertices[vertices.size() - 1].normal = normals[i];
        }

        for (int i = 0; i < vertices.size(); i++) {
            vertices[i].entity_id = (uint32_t)entity_id;
        }

        std::vector<uint32_t> indices = {
            // Front face
            0, 1, 8, 1, 2, 8, 2, 3, 8, 3, 0, 8,
            // Right face
            1, 5, 9, 5, 6, 9, 6, 2, 9, 2, 1, 9,
            // Back face
            5, 4, 10, 4, 7, 10, 7, 6, 10, 6, 5, 10,
            // Left face
            4, 0, 11, 0, 3, 11, 3, 7, 11, 7, 4, 11,
            // Top face
            3, 2, 12, 2, 6, 12, 6, 7, 12, 7, 3, 12,
            // Bottom face
            4, 5, 13, 5, 1, 13, 1, 0, 13, 0, 4, 13
        };

        mesh = Engine::CreateRef<Engine::Mesh>(vertices, indices);
	}
}