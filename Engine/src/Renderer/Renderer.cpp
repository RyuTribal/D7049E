#include "pch.h"
#include "Renderer.h"
#include "Core/Application.h"
#include "Framebuffer.h"

namespace Engine
{
    Renderer* Renderer::s_Instance = nullptr;

    Renderer::Renderer()
    {
		m_RendererAPI.Init();
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

		TextureSpecification texture_spec{};
		texture_spec.Width = 1024;
		texture_spec.Height = 1024;
		texture_spec.Format = ImageFormat::RGBA8;

		m_DefaultMap = CreateRef<Texture2D>(texture_spec);

		uint32_t bpp = 4;
		size_t dataSize = texture_spec.Width * texture_spec.Height * bpp;
		auto* data = new uint8_t[dataSize];

		std::fill_n(data, dataSize, 0);
		m_DefaultMap->SetData(data, dataSize);

		delete[] data;



		ReCreateFrameBuffers();
    }

    Renderer::~Renderer() {
        
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

		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE18);
		m_RendererAPI.BindTexture(depthTextureID);
		m_LightCullingProgram.UploadIntData("depthMap", 18);


		m_RendererAPI.DispatchCompute(m_WorkGroupsX, m_WorkGroupsY, 1);
		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE18);

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
		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE19);
		m_RendererAPI.BindTexture(color_attachment);
		m_QuadProgram.UploadFloatData("exposure", exposure);
		m_QuadProgram.UploadIntData("hdrBuffer", 19);

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
				material->GetProgram()->UploadMat4FloatData("u_Transform", mesh->GetTransform() * mesh->GetSubmeshes()[i].WorldTransform);
				material->GetProgram()->UploadMat4FloatData("u_CameraView", m_CurrentCamera->GetView());
				material->GetProgram()->UploadMat4FloatData("u_CameraProjection", m_CurrentCamera->GetProjection());
				material->GetProgram()->UploadIntData("u_NumDirectionalLights", (int)m_DirectionalLights.size());
			}
			m_RendererAPI.DrawIndexed(mesh->GetSubmeshes()[i].VertexArray);
			m_Stats.draw_calls++;
		}
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
