#include "pch.h"
#include "Renderer.h"
#include "Core/Application.h"
#include "Framebuffer.h"

namespace Engine
{
	struct DefaultTextures
	{
		Ref<Texture2D> White;
		Ref<Texture2D> Black;
		Ref<Texture2D> Gray;
		Ref<Texture2D> Blue;
	};
	
	static DefaultTextures* s_DefaultTextures;

    Renderer* Renderer::s_Instance = nullptr;

    Renderer::Renderer()
    {
		m_RendererAPI.Init();

		s_DefaultTextures = new DefaultTextures();

		TextureSpecification default_texture_spec;
		default_texture_spec.Format = ImageFormat::RGBA8;
		default_texture_spec.Width = 1;
		default_texture_spec.Height = 1;

		uint32_t whiteTextureData = 0xffffffff;
		s_DefaultTextures->White = Texture2D::Create(default_texture_spec, Buffer(&whiteTextureData, sizeof(uint32_t)));

		uint32_t blackTextureData = 0xff000000;
		s_DefaultTextures->Black = Texture2D::Create(default_texture_spec, Buffer(&whiteTextureData, sizeof(uint32_t)));

		uint32_t grayTextureData = 0xff808080;
		s_DefaultTextures->Gray = Texture2D::Create(default_texture_spec, Buffer(&whiteTextureData, sizeof(uint32_t)));

		uint32_t blueTextureData = 0xffff8080;
		s_DefaultTextures->Blue = Texture2D::Create(default_texture_spec, Buffer(&whiteTextureData, sizeof(uint32_t)));


		current_window_width = Application::Get().GetWindow().GetWidth();
		current_window_height = Application::Get().GetWindow().GetHeight();
		FramebufferSpecification depthSpec = {};
		depthSpec.Width = current_window_width;
		depthSpec.Height = current_window_height;
		depthSpec.Attachments = { FramebufferTextureFormat::DEPTH24STENCIL8 };
		m_DepthFramebuffer = Engine::Framebuffer::Create(depthSpec);

		FramebufferSpecification hdrSpec = {};
		hdrSpec.Width = current_window_width;
		hdrSpec.Height = current_window_height;
		hdrSpec.Attachments = {
				FramebufferTextureFormat::RGBA16F,
				FramebufferTextureFormat::DEPTH24STENCIL8
		};
		m_HDRFramebuffer = Engine::Framebuffer::Create(hdrSpec);

		FramebufferSpecification sceneSpec = {};
		sceneSpec.Width = current_window_width;
		sceneSpec.Height = current_window_height;
		sceneSpec.Attachments = {
				FramebufferTextureFormat::RGBA16F,
				FramebufferTextureFormat::DEPTH24STENCIL8
		};
		m_SceneFramebuffer = Engine::Framebuffer::Create(sceneSpec);

		m_LightsSSBO = CreateRef<ShaderStorageBuffer>(sizeof(PointLightInfo) * MAX_POINT_LIGHTS, 2);
		m_DirLightsSSBO = CreateRef<ShaderStorageBuffer>(sizeof(DirectionalLightInfo) * MAX_DIR_LIGHTS, 0);

		m_ShaderLibrary.Load("default_static_pbr", "Resources/Shaders/default_static_shader");
		m_ShaderLibrary.Load("forward_plus_depth_pre_pass", "Resources/Shaders/depth_pre_pass");
		m_ShaderLibrary.Load("forward_plus_light_culling", "Resources/Shaders/light_culling_shader");
		m_ShaderLibrary.Load("hdr_shader", "Resources/Shaders/hdr_shader");
		m_ShaderLibrary.Load("line_shader", "Resources/Shaders/line");

		ReCreateFrameBuffers();
    }

    Renderer::~Renderer() {
		delete s_DefaultTextures;
    }

	void Renderer::SubmitObject(Ref<Mesh> mesh)
	{
		HVE_PROFILE_FUNC();
		m_Meshes.push_back(mesh);
		m_Stats.vertices_count += mesh->GetMeshSource()->VertexSize();
		m_Stats.index_count += mesh->GetMeshSource()->IndexSize();
	}

	void Renderer::SubmitDebugLine(Line line)
	{
		m_DebugLines.push_back(line);
	}

	void Renderer::SubmitDebugBox(DebugBox box)
	{
		m_DebugBoxes.push_back(box);
	}

	void Renderer::SubmitDebugSphere(DebugSphere sphere)
	{
		m_DebugSpheres.push_back(sphere);
	}

	void Renderer::BeginFrame(Camera* camera)
    {
		HVE_PROFILE_FUNC();
		auto& app = Application::Get();
		float r, g, b;
		r = m_BackgroundColor[0] / 255.0f;
		g = m_BackgroundColor[1] / 255.0f;
		b = m_BackgroundColor[2] / 255.0f;

		m_RendererAPI.SetClearColor(glm::vec4(r, g, b, 1.f));

		if (m_CurrentCamera) {
			camera->SetAspectRatio(m_CurrentCamera->GetAspectRatio());
		}
        SetCamera(camera);
        camera->UpdateCamera();

		m_RendererAPI.UnBindBuffer();
		ResetStats();
    }

	void Renderer::DepthPrePass()
	{
		HVE_PROFILE_FUNC();
		m_RendererAPI.ClearDepth();
		m_DepthFramebuffer->Bind();
		Ref<ShaderProgram> shader = m_ShaderLibrary.Get("forward_plus_depth_pre_pass");
		shader->Set("u_CameraView", m_CurrentCamera->GetView());
		shader->Set("u_CameraProjection", m_CurrentCamera->GetProjection());
		shader->Activate();
		for (Ref<Mesh> mesh : m_Meshes) {
			DrawIndexed(mesh, false);
		}

		m_DepthFramebuffer->Unbind();
	}

	void Renderer::CullLights()
	{
		HVE_PROFILE_FUNC();
		Ref<ShaderProgram> shader = m_ShaderLibrary.Get("forward_plus_light_culling");
		shader->Set("lightCount", (int)m_PointLights.size());
		shader->Set("screenSize", glm::ivec2((int)current_window_width, (int)current_window_height));
		shader->Set("view", m_CurrentCamera->GetView());
		shader->Set("projection", m_CurrentCamera->GetProjection());
		shader->Activate();

		uint32_t depthTextureID = m_DepthFramebuffer->GetDepthAttachmentID();

		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE4);
		m_RendererAPI.BindTexture(depthTextureID);


		m_RendererAPI.DispatchCompute(m_WorkGroupsX, m_WorkGroupsY, 1);
		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE4);

		m_RendererAPI.UnBindTexture(depthTextureID);
	}

	void Renderer::ShadeAllObjects()
	{
		HVE_PROFILE_FUNC();
		for (size_t i = 0; i < m_Meshes.size(); i++) {
			DrawIndexed(m_Meshes[i], true);
		}
		
	}

	void Renderer::ShadeHDR()
	{
		HVE_PROFILE_FUNC();
		Ref<ShaderProgram> shader = m_ShaderLibrary.Get("hdr_shader");
		shader->Activate();

		uint32_t color_attachment = m_HDRFramebuffer->GetColorAttachmentRendererID();
		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE0);
		m_RendererAPI.BindTexture(color_attachment);
		shader->Set("exposure", exposure);
		shader->Activate();

		m_SceneFramebuffer->Bind();
		m_RendererAPI.ClearAll();
		DrawHDRQuad();
		m_SceneFramebuffer->Unbind();
	}

	void Renderer::DrawIndexed(Ref<Mesh> mesh, bool use_material)
	{
		HVE_PROFILE_FUNC();
		for (size_t i = 0; i < mesh->GetMeshSource()->GetSubmeshes().size(); i++)
		{
			Ref<Material> material = mesh->GetMeshSource()->GetMaterials()[mesh->GetMeshSource()->GetSubmeshes()[i].MaterialIndex];
			if (use_material)
			{
				material->Set("u_CameraPos", GetCamera()->CalculatePosition());
				material->Set("u_CameraView", GetCamera()->GetView());
				material->Set("u_CameraProjection", Renderer::Get()->GetCamera()->GetProjection());
				material->Set("u_Transform", mesh->GetTransform() * mesh->GetMeshSource()->GetSubmeshes()[i].WorldTransform);
				material->Set("u_NumDirectionalLights", (int)m_DirectionalLights.size());
				material->ApplyMaterial();
			}
			m_RendererAPI.DrawIndexed(mesh->GetMeshSource()->GetSubmeshes()[i].VertexArray);
			m_Stats.draw_calls++;
		}
	}

	Ref<Texture2D> Renderer::GetWhiteTexture()
	{
		return s_DefaultTextures->White;
	}

	Ref<Texture2D> Renderer::GetBlackTexture()
	{
		return s_DefaultTextures->Black;
	}

	Ref<Texture2D> Renderer::GetGrayTexture()
	{
		return s_DefaultTextures->Gray;
	}

	Ref<Texture2D> Renderer::GetBlueTexture()
	{
		return s_DefaultTextures->Blue;
	}

	void Renderer::BeginDrawing()
	{
		DepthPrePass();
		UploadLightData();
		CullLights();

		m_HDRFramebuffer->Bind();
		m_RendererAPI.ClearAll();
		ShadeAllObjects();

		m_RendererAPI.SetDepthWriting(false);
		DrawDebugObjects();
		m_RendererAPI.SetDepthWriting(true);

		m_HDRFramebuffer->Unbind();
		m_RendererAPI.ClearAll();

		ShadeHDR();
	}

    void Renderer::EndFrame()
    {
		glUseProgram(0);
		m_Meshes.clear();
		m_PointLights.clear();
		m_DirectionalLights.clear();
		m_DebugLines.clear();
		m_DebugBoxes.clear();
		m_DebugSpheres.clear();
    }

	void Renderer::ReCreateFrameBuffers()
	{
		HVE_PROFILE_FUNC();
		
		m_DepthFramebuffer->Resize(current_window_width, current_window_height);
		m_HDRFramebuffer->Resize(current_window_width, current_window_height);
		m_SceneFramebuffer->Resize(current_window_width, current_window_height);

		m_WorkGroupsX = (current_window_width + ((int)current_window_width % 16)) / 16;
		m_WorkGroupsY = (current_window_height + ((int)current_window_height % 16)) / 16;
		size_t numberOfTiles = m_WorkGroupsX * m_WorkGroupsY;
		size_t data = numberOfTiles * sizeof(VisibleIndex) * 1024;
		m_VisibleLightsSSBO = CreateRef<ShaderStorageBuffer>(data, 1);
		m_VisibleLightsSSBO->Bind();
	}
	void Renderer::UploadLightData() {

		m_LightsSSBO->Bind();

		std::vector<PointLightInfo> pointLightsData(m_PointLights.size());
		for (size_t i = 0; i < m_PointLights.size(); ++i) {
			pointLightsData[i].color = glm::vec4(m_PointLights[i]->GetColor(), 1.f);
			pointLightsData[i].intensity = m_PointLights[i]->GetIntensity();
			pointLightsData[i].position = glm::vec4(m_PointLights[i]->GetPosition(), 1.f);
			pointLightsData[i].constantAttenuation = m_PointLights[i]->GetConstantAttenuation();
			pointLightsData[i].linearAttenuation = m_PointLights[i]->GetLinearAttenuation();
			pointLightsData[i].quadraticAttenuation = m_PointLights[i]->GetQuadraticAttenuation();
		}

		m_LightsSSBO->SetData(pointLightsData.data(), pointLightsData.size() * sizeof(PointLightInfo));

		m_DirLightsSSBO->Bind();

		std::vector<DirectionalLightInfo> dirLightsData(m_DirectionalLights.size());
		for (size_t i = 0; i < m_DirectionalLights.size(); ++i)
		{
			dirLightsData[i].color = glm::vec4(m_DirectionalLights[i]->GetColor(), 1.f);
			dirLightsData[i].direction = glm::vec4(m_DirectionalLights[i]->GetDirection(), 1.f);
			dirLightsData[i].intensity = m_DirectionalLights[i]->GetIntensity();
		}
		m_DirLightsSSBO->SetData(dirLightsData.data(), dirLightsData.size() * sizeof(DirectionalLightInfo));
	}
	void Renderer::DrawHDRQuad()
	{
		HVE_PROFILE_FUNC();
		// have to fix this later, want to get rid of native gl calls from the renderer
		if (m_QuadVAO == 0) {
			GLfloat quadVertices[] = {
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};

			glGenVertexArrays(1, &m_QuadVAO);
			glGenBuffers(1, &m_QuadVBO);
			glBindVertexArray(m_QuadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		}

		glBindVertexArray(m_QuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	void Renderer::DrawDebugObjects()
	{
		for (auto& box : m_DebugBoxes)
		{
			glm::vec3 vertices[] = {
			glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f, 1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(1.0f, -1.0f, 1.0f),
			glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)
			};

			int edges[][2] = {
				{0, 1}, {1, 3}, {3, 2}, {2, 0},
				{4, 5}, {5, 7}, {7, 6}, {6, 4},
				{0, 4}, {1, 5}, {2, 6}, {3, 7}
			};

			glm::mat4 transform = glm::scale(box.Transform, box.Size);

			for (auto& edge : edges)
			{
				m_DebugLines.push_back(Line{ vertices[edge[0]], vertices[edge[1]], box.Color, transform });
			}
		}

		const int latitudeDivisions = 12;
		const int longitudeDivisions = 12;

		for (auto& sphere : m_DebugSpheres)
		{
			std::vector<glm::vec3> vertices;

			for (int i = 0; i <= latitudeDivisions; ++i)
			{
				float lat = glm::pi<float>() * i / latitudeDivisions;

				for (int j = 0; j <= longitudeDivisions; ++j)
				{
					float lon = 2.0f * glm::pi<float>() * j / longitudeDivisions;

					float x = sphere.Radius * sin(lat) * cos(lon);
					float y = sphere.Radius * sin(lat) * sin(lon);
					float z = sphere.Radius * cos(lat);

					glm::vec3 pos = glm::vec3(sphere.Transform * glm::vec4(x, y, z, 1.0f));
					vertices.push_back(pos);
				}
			}

			for (int i = 0; i < latitudeDivisions; ++i)
			{
				for (int j = 0; j < longitudeDivisions; ++j)
				{
					m_DebugLines.push_back(Line{ vertices[i * (longitudeDivisions + 1) + j], vertices[i * (longitudeDivisions + 1) + j + 1], sphere.Color, glm::mat4(1.0f) });
					m_DebugLines.push_back(Line{ vertices[i * (longitudeDivisions + 1) + j], vertices[(i + 1) * (longitudeDivisions + 1) + j], sphere.Color, glm::mat4(1.0f) });
				}
			}
		}

		Ref<ShaderProgram> shader = m_ShaderLibrary.Get("line_shader");
		shader->Set("u_CameraView", m_CurrentCamera->GetView());
		shader->Set("u_CameraProjection", m_CurrentCamera->GetProjection());
		shader->Activate();
		m_RendererAPI.DrawInstancedLines(m_DebugLines);
	}

	void Renderer::ResizeViewport(int width, int height)
	{
		if (width == current_window_width && height == current_window_height)
		{
			return;
		}
		current_window_width = width;
		current_window_height = height ? height : 1;
		m_CurrentCamera->SetAspectRatio(current_window_width / current_window_height);
		m_RendererAPI.SetViewport(0, 0, current_window_width, current_window_height);
		ReCreateFrameBuffers();
	}
	void Renderer::SetVSync(bool vsync)
	{
		auto& app = Application::Get();
		app.GetWindow().GetContext()->SetVSync(vsync);
	}
	void Renderer::SetViewport(int width, int height)
	{
		m_RendererAPI.SetViewport(0, 0, width, height);
	}
	void Renderer::ResetStats()
	{
		m_Stats.vertices_count = 0;
		m_Stats.draw_calls = 0;
		m_Stats.index_count = 0;
	}
}
