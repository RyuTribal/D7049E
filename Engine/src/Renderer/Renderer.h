#pragma once
#include "Camera.h"
#include <glad/gl.h>
#include "ShaderProgram.h"
#include <Lights/PointLight.h>
#include "Mesh.h"
#include "Material.h"
#include <Scene/Components.h>
#include "RendererAPI.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "Framebuffer.h"
#include "Shader.h"

namespace Engine
{

	struct GLFWwindow;

	const int MAX_POINT_LIGHTS = 1000;

	const int MAX_DIR_LIGHTS = 2;

	struct PointLightInfo {
		float constantAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;
		float intensity;
		glm::vec4 color; // vec4 necessary for GLSL allignment
		glm::vec4 position;
	};

	struct DebugBox
	{
		glm::vec3 Size;
		glm::mat4 Transform;
		glm::vec4 Color;
	};

	struct DebugSphere
	{
		float Radius;
		glm::mat4 Transform;
		glm::vec4 Color;
	};

	struct DirectionalLightInfo
	{
		glm::vec3 padding = { 1.f, 1.f, 1.f }; // I honestly haven't come up with an answer as to why the data corrupts if I don't have this padding. Something to do with allignment
		float intensity;
		glm::vec4 color;
		glm::vec4 direction;
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

		int draw_calls = 0;
		int vertices_count = 0;
		int index_count = 0;

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

		void SubmitObject(Ref<Mesh> mesh);
		void SubmitPointLight(PointLight* point_light) { m_PointLights.push_back(point_light); }
		void SubmitDirectionalLight(DirectionalLight* light) { m_DirectionalLights.push_back(light); }

		void SubmitDebugLine(Line line);
		void SubmitDebugBox(DebugBox box);
		void SubmitDebugSphere(DebugSphere sphere);

		void BeginFrame(Camera* camera);
		void DrawIndexed(Ref<Mesh> mesh, bool use_material);


		static Ref<Texture2D> GetWhiteTexture();
		static Ref<Texture2D> GetBlackTexture();
		static Ref<Texture2D> GetGrayTexture();
		static Ref<Texture2D> GetBlueTexture();

		static ShaderLibrary* GetShaderLibrary()
		{
			return &Get()->m_ShaderLibrary;
		}

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
		uint32_t GetSceneTextureID() { return m_SceneFramebuffer->GetColorAttachmentRendererID(); }
		Ref<Framebuffer> GetObjectFrameBuffer() { m_HDRFramebuffer->Bind(); return m_HDRFramebuffer; }

		Statistics* GetStats() { return &m_Stats; }
		void ResizeViewport(int width, int height);
		void SetVSync(bool vsync);
		void SetViewport(int width, int height);
		void BindTextureUnit(TextureUnits unit) { m_RendererAPI.ActivateTextureUnit(unit); }

		void SetDrawBoundingBoxes(bool should_draw) { m_DrawBoundingBox = should_draw; }

	private:

		void DepthPrePass();
		void CullLights();
		void ShadeAllObjects();
		void ShadeHDR();

		void ResetStats();
		void ReCreateFrameBuffers();
		void UploadLightData();
		void DrawHDRQuad();

		void DrawDebugObjects();

		ShaderLibrary m_ShaderLibrary{};

		static Renderer* s_Instance;
		Camera* m_CurrentCamera = nullptr;

		GLuint m_WorkGroupsX;
		GLuint m_WorkGroupsY;

		Ref<Framebuffer> m_DepthFramebuffer = nullptr;
		Ref<ShaderStorageBuffer> m_LightsSSBO = nullptr;
		Ref<ShaderStorageBuffer> m_DirLightsSSBO = nullptr;
		Ref<ShaderStorageBuffer> m_VisibleLightsSSBO = nullptr;
		Ref<Framebuffer> m_HDRFramebuffer = nullptr;
		Ref<Framebuffer> m_SceneFramebuffer = nullptr;

		int m_BackgroundColor[3] = { 0, 0, 0 };

		Statistics m_Stats{};
		
		float current_window_width, current_window_height;

		std::vector<Ref<Mesh>> m_Meshes{};
		std::vector<PointLight*> m_PointLights{};
		std::vector<DirectionalLight*> m_DirectionalLights{};

		std::vector<Line> m_DebugLines{};
		std::vector<DebugBox> m_DebugBoxes{};
		std::vector<DebugSphere> m_DebugSpheres{};
		
		Ref<VertexArray> m_QuadVertexArray = nullptr;

		const float exposure = 1.0f;

		GLuint m_QuadVAO = 0;
		GLuint m_QuadVBO;

		RendererAPI m_RendererAPI{};

		bool m_DrawBoundingBox = false;
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
