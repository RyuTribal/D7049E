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

		ReCreateFrameBuffers();
    }

    Renderer::~Renderer() {
        
    }

    void Renderer::BeginFrame(Camera* camera)
    {
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
    }

	void Renderer::DepthPrePass()
	{
		m_RendererAPI.ClearDepth();
		m_DepthPrePassProgram.Activate();

		m_DepthPrePassProgram.UploadMat4FloatData("u_CameraView", m_CurrentCamera->GetView());
		m_DepthPrePassProgram.UploadMat4FloatData("u_CameraProjection", m_CurrentCamera->GetProjection());
		for (Mesh* mesh : m_Meshes) {
			DrawIndexed(mesh, nullptr);
		}

		m_DepthFramebuffer->Unbind();
	}

	void Renderer::CullLights()
	{
		m_LightCullingProgram.Activate();
		m_LightCullingProgram.UploadIntData("lightCount", (int)m_PointLights.size());
		m_LightCullingProgram.UploadVec2IntData("screenSize", glm::ivec2((int)current_window_width, (int)current_window_height));
		m_LightCullingProgram.UploadMat4FloatData("view", m_CurrentCamera->GetView());
		m_LightCullingProgram.UploadMat4FloatData("projection", m_CurrentCamera->GetProjection());

		uint32_t depthTextureID = m_DepthFramebuffer->GetDepthAttachmentID();

		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE4);
		m_RendererAPI.BindTexture(depthTextureID);
		m_LightCullingProgram.UploadIntData("depthMap", 4);

		m_LightsSSBO->Bind();
		m_VisibleLightsSSBO->Bind();

		m_RendererAPI.DispatchCompute(m_WorkGroupsX, m_WorkGroupsY, 1);
		m_RendererAPI.ActivateTextureUnit(TextureUnits::TEXTURE4);
		m_RendererAPI.UnBindTexture(depthTextureID);
	}

	void Renderer::ShadeAllObjects()
	{
		m_HDRFramebuffer->Bind();
		m_RendererAPI.ClearAll();

		for (size_t i = 0; i < m_Meshes.size(); i++) {
			DrawIndexed(m_Meshes[i], m_Materials[i]);
		}
		m_HDRFramebuffer->Unbind();

		uint32_t color_attachment = m_HDRFramebuffer->GetColorAttachmentRendererID();

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // need to create an abstraction for this in renderer API

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
		
		m_LightsSSBO->Unbind();
		m_VisibleLightsSSBO->Unbind();
	}

	void Renderer::DrawIndexed(Mesh* mesh, Material* material)
	{
		if (material != nullptr) {
			material->ApplyMaterial();
			material->GetProgram()->UploadMat4FloatData("u_Transform", mesh->GetTransform());
			material->GetProgram()->UploadMat4FloatData("u_CameraView", m_CurrentCamera->GetView());
			material->GetProgram()->UploadMat4FloatData("u_CameraProjection", m_CurrentCamera->GetProjection());
		}
		m_RendererAPI.DrawIndexed(mesh->GetVertexArray());
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
		m_Materials.clear();
    }

	void Renderer::ReCreateFrameBuffers()
	{
		m_WorkGroupsX = (current_window_width + ((int)current_window_width % 16)) / 16;
		m_WorkGroupsY = (current_window_height + ((int)current_window_height % 16)) / 16;
		
		m_DepthFramebuffer->Resize(current_window_width, current_window_height);
		m_HDRFramebuffer->Resize(current_window_width, current_window_height);
		m_SceneFramebuffer->Resize(current_window_width, current_window_height);

		size_t numberOfTiles = m_WorkGroupsX * m_WorkGroupsY;
		size_t data = numberOfTiles * sizeof(VisibleIndex) * 1024;
		m_VisibleLightsSSBO = CreateRef<ShaderStorageBuffer>(data, 1);
		m_VisibleLightsSSBO->Bind();
	}
	void Renderer::UploadLightData() {
		if (!m_LightsSSBO) {
			m_LightsSSBO = CreateRef<ShaderStorageBuffer>(sizeof(PointLightInfo) * m_PointLights.size(), 0);
		}

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
	}
	void Renderer::DrawHDRQuad()
	{
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
	void Renderer::ResetStats()
	{
		m_Stats = Statistics();
	}
}