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
		s_DefaultTextures->White = Texture2D::Create(default_texture_spec);
		s_DefaultTextures->White->SetData(&whiteTextureData, sizeof(whiteTextureData));

		uint32_t blackTextureData = 0xff000000;
		s_DefaultTextures->Black = Texture2D::Create(default_texture_spec);
		s_DefaultTextures->Black->SetData(&blackTextureData, sizeof(blackTextureData));

		uint32_t grayTextureData = 0xff808080;
		s_DefaultTextures->Gray = Texture2D::Create(default_texture_spec);
		s_DefaultTextures->Gray->SetData(&grayTextureData, sizeof(grayTextureData));

		uint32_t blueTextureData = 0xffff8080;
		s_DefaultTextures->Blue = Texture2D::Create(default_texture_spec);
		s_DefaultTextures->Blue->SetData(&blueTextureData, sizeof(blueTextureData));


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
				FramebufferTextureFormat::RED_INTEGER,
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

		m_LightsSSBO = CreateRef<ShaderStorageBuffer>(sizeof(PointLightInfo) * MAX_POINT_LIGHTS, 2); // Maybe create a resize shader buffer function if possible
		m_DirLightsSSBO = CreateRef<ShaderStorageBuffer>(sizeof(DirectionalLightInfo) * MAX_DIR_LIGHTS, 0); // Maybe create a resize shader buffer function if possible

		ReCreateFrameBuffers();
    }

    Renderer::~Renderer() {
		delete s_DefaultTextures;
    }

	void Renderer::SubmitObject(Mesh* mesh)
	{
		HVE_PROFILE_FUNC();
		m_Meshes.push_back(mesh);
		m_Stats.vertices_count += mesh->VertexSize();
		m_Stats.index_count += mesh->IndexSize();
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
		m_DepthPrePassProgram.Activate();
		m_DepthPrePassProgram.UploadMat4FloatData("u_CameraView", m_CurrentCamera->GetView());
		m_DepthPrePassProgram.UploadMat4FloatData("u_CameraProjection", m_CurrentCamera->GetProjection());
		for (Mesh* mesh : m_Meshes) {
			DrawIndexed(mesh, false);
		}

		m_DepthFramebuffer->Unbind();
	}

	void Renderer::CullLights()
	{
		HVE_PROFILE_FUNC();
		m_LightCullingProgram.Activate();
		m_LightCullingProgram.UploadIntData("lightCount", (int)m_PointLights.size());
		m_LightCullingProgram.UploadVec2IntData("screenSize", glm::ivec2((int)current_window_width, (int)current_window_height));
		m_LightCullingProgram.UploadMat4FloatData("view", m_CurrentCamera->GetView());
		m_LightCullingProgram.UploadMat4FloatData("projection", m_CurrentCamera->GetProjection());

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
		m_HDRFramebuffer->Bind();
		m_HDRFramebuffer->ClearAttachment(1, -1);
		m_RendererAPI.ClearAll();

		for (size_t i = 0; i < m_Meshes.size(); i++) {
			DrawIndexed(m_Meshes[i], true);
		}

		m_HDRFramebuffer->Unbind();


		uint32_t color_attachment = m_HDRFramebuffer->GetColorAttachmentRendererID();

		m_RendererAPI.ClearAll();
		m_QuadProgram.Activate();

		color_attachment = m_HDRFramebuffer->GetColorAttachmentRendererID();
		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE0);
		m_RendererAPI.BindTexture(color_attachment);
		m_QuadProgram.UploadFloatData("exposure", exposure);

		m_SceneFramebuffer->Bind();
		m_RendererAPI.ClearAll();
		DrawHDRQuad();
		m_SceneFramebuffer->Unbind();
		
	}

	void Renderer::DrawIndexed(Mesh* mesh, bool use_material)
	{
		HVE_PROFILE_FUNC();
		for (size_t i = 0; i < mesh->GetSubmeshes().size(); i++)
		{
			
			if (use_material)
			{
				Ref<Material> material = mesh->GetMaterials()[mesh->GetSubmeshes()[i].MaterialIndex];
				material->ApplyMaterial();
				material->GetProgram()->UploadVec3FloatData("u_CameraPos", m_CurrentCamera->CalculatePosition());
				material->GetProgram()->UploadMat4FloatData("u_Transform", mesh->GetTransform() * mesh->GetSubmeshes()[i].WorldTransform);
				material->GetProgram()->UploadMat4FloatData("u_CameraView", m_CurrentCamera->GetView());
				material->GetProgram()->UploadMat4FloatData("u_CameraProjection", m_CurrentCamera->GetProjection());
				material->GetProgram()->UploadIntData("u_NumDirectionalLights", (int)m_DirectionalLights.size());
			}
			m_RendererAPI.DrawIndexed(mesh->GetSubmeshes()[i].VertexArray);
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
		ShadeAllObjects();
	}

    void Renderer::EndFrame()
    {
		glUseProgram(0);
		m_Meshes.clear();
		m_PointLights.clear();
		m_DirectionalLights.clear();
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
	void Renderer::ResizeViewport(int width, int height)
	{
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
