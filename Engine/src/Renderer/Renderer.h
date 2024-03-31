#pragma once
#include "Camera.h"
#include <glad/gl.h>
#include "ShaderProgram.h"
#include <Lights/PointLight.h>
#include "Mesh.h"
#include "Material.h"
#include <Scene/Components.h>

namespace Engine
{

	struct GLFWwindow;

	struct PointLightInfo {
		float constantAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;
		float intensity;
		glm::vec4 color; // vec4 necessary for GLSL allignment
		glm::vec4 position;
	};

	struct VisibleIndex {
		int index;
	};

	struct TextureInfo {
		GLuint texture;
		int height;
		int width;
		int channel_number;
	};

	struct Statistics {
		double frames_per_second = 0.0;
		double frame_time_accumulator = 0.0;
		int frame_count = 0;
		double last_FPS_calculation_time = 0.0;

		void UpdateFPS(double currentTime, double frameTime) {
			frame_time_accumulator += frameTime;
			frame_count++;

			if (currentTime - last_FPS_calculation_time >= 1.0) {
				double avgFrameTime = frame_time_accumulator / frame_count;
				frames_per_second = 1.0 / avgFrameTime;

				frame_time_accumulator = 0.0;
				frame_count = 0;
				last_FPS_calculation_time = currentTime;
			}
		}
	};

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void SubmitObject(Mesh* mesh, Material* material) { m_Meshes.push_back(mesh); m_Materials.push_back(material); }
		void SubmitPointLight(PointLight* point_light) { m_PointLights.push_back(point_light); }

		void BeginFrame(Camera* camera);

		void DepthPrePass();
		void CullLights();
		void ShadeAllObjects();
		void DrawIndexed(Mesh* mesh, Material* material);

		void EndFrame();

		void BeginDrawing();

		static void CreateRenderer()
		{
			if(!s_Instance)
			{
				s_Instance = new Renderer();
			}
		}
		static Renderer* Get() { return s_Instance; }

		Camera* GetCamera() { return m_CurrentCamera; }
		void SetCamera(Camera* camera) { m_CurrentCamera = camera; }

		void SetBackgroundColor(int red, int green, int blue) { m_BackgroundColor[0] = red; m_BackgroundColor[1] = green; m_BackgroundColor[2] = blue;}

		TextureInfo UploadImageToGPU(const char* path, bool invert);

		Statistics* GetStats() { return &m_Stats; }
		bool OnWindowResized();
		void SetVSync(bool vsync);
	private:

		void ResetStats();
		void ReCreateFrameBuffers();
		void UploadLightData();
		void DrawHDRQuad();

		static Renderer* s_Instance;
		Camera* m_CurrentCamera = nullptr;

		GLuint m_WorkGroupsX;
		GLuint m_WorkGroupsY;

		GLuint m_LightsBuffer;
		GLuint m_VisibleLightsBuffer;

		GLuint m_DepthTexture;
		GLuint m_DepthFBO;

		GLuint m_HDRFBO;
		GLuint m_ColorBuffer;
		GLuint m_RBODepth;
		ShaderProgram m_DepthPrePassProgram = ShaderProgram(std::string(ROOT_PATH) + "/shaders/forward_plus/depth_pre_pass");

		ShaderProgram m_LightCullingProgram = ShaderProgram(std::string(ROOT_PATH) + "/shaders/forward_plus/light_culling_shader");

		int m_BackgroundColor[3] = { 0, 0, 0 };

		Statistics m_Stats{};
		
		float current_window_width, current_window_height;

		std::vector<Mesh*> m_Meshes{};
		std::vector<Material*> m_Materials{};
		std::vector<PointLight*> m_PointLights{};

		GLuint m_QuadVAO = 0;
		GLuint m_QuadVBO;

		ShaderProgram m_QuadProgram = ShaderProgram(std::string(ROOT_PATH) + "/shaders/forward_plus/hdr_shader");

		const float exposure = 1.0f;
	};

	// This is so the spd log library can print this data structure
	inline std::ostream& operator<<(std::ostream& os, const Vertex& v) {
		return os << "{" << "x: " << v.coordinates.x << ", y: " << v.coordinates.y << ", z: " << v.coordinates.z << "}";
	}

	inline std::ostream& operator<<(std::ostream& os, const std::vector<Vertex>& vec) {
		os << "[ \n";
		int row_items_count = 0;
		const int max_row_items = 2;
		for (size_t i = 0; i < vec.size(); ++i) {
			if(row_items_count == 0)
			{
				os << "\t";
			}
			os << vec[i];
			if (i < vec.size() - 1) {
				os << ", ";
				row_items_count++;
				if(row_items_count >= max_row_items)
				{
					os << "\n";
					row_items_count = 0;
				}
			}
		}

		os << "\n";

		return os << "]";
	}

}
