#include "pch.h"
#include "Renderer.h"
#include "Core/Application.h"

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine
{
    Renderer* Renderer::s_Instance = nullptr;

    Renderer::Renderer()
    {
		glGenBuffers(1, &m_LightsBuffer);
		glGenBuffers(1, &m_VisibleLightsBuffer);
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

		glClearColor(r, g, b, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (m_CurrentCamera) {
			camera->SetAspectRatio(m_CurrentCamera->GetAspectRatio());
		}
        SetCamera(camera);
        camera->UpdateCamera();
    }

	void Renderer::DepthPrePass()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		glUseProgram(m_DepthPrePassProgram.GetProgram());
		m_DepthPrePassProgram.UploadMat4FloatData("u_CameraView", m_CurrentCamera->GetView());
		m_DepthPrePassProgram.UploadMat4FloatData("u_CameraProjection", m_CurrentCamera->GetProjection());
		for (Mesh* mesh : m_Meshes) {
			DrawIndexed(mesh, nullptr);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::CullLights()
	{
		glUseProgram(m_LightCullingProgram.GetProgram());
		m_LightCullingProgram.UploadIntData("lightCount", (int)m_PointLights.size());
		m_LightCullingProgram.UploadVec2IntData("screenSize", glm::ivec2((int)current_window_width, (int)current_window_height));
		m_LightCullingProgram.UploadMat4FloatData("view", m_CurrentCamera->GetView());
		m_LightCullingProgram.UploadMat4FloatData("projection", m_CurrentCamera->GetProjection());
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(m_LightCullingProgram.GetProgram(), "depthMap"), 4);
		glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_LightsBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_VisibleLightsBuffer);
		glDispatchCompute(m_WorkGroupsX, m_WorkGroupsY, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Renderer::ShadeAllObjects()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (size_t i = 0; i < m_Meshes.size(); i++) {
			DrawIndexed(m_Meshes[i], m_Materials[i]);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_QuadProgram.Activate();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ColorBuffer);
		glUniform1f(glGetUniformLocation(m_QuadProgram.GetProgram(), "exposure"), exposure);
		DrawHDRQuad();

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	}

	void Renderer::DrawIndexed(Mesh* mesh, Material* material)
	{
		if (material != nullptr) {
			material->ApplyMaterial();
			material->GetProgram()->UploadMat4FloatData("u_Transform", mesh->GetTransform());
			material->GetProgram()->UploadMat4FloatData("u_CameraView", m_CurrentCamera->GetView());
			material->GetProgram()->UploadMat4FloatData("u_CameraProjection", m_CurrentCamera->GetProjection());
		}
		mesh->BindData();
		glDrawElements(GL_TRIANGLES, (GLsizei)mesh->GetIndices().size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
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
        glBindVertexArray(0);
		m_Meshes.clear();
		m_PointLights.clear();
		m_Materials.clear();
    }


	TextureInfo Renderer::UploadImageToGPU(const char* path, bool invert)
	{
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(invert);
		unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
		if (data)
		{
			if (nrChannels == 4) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else if (nrChannels == 3) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			CORE_ERROR("Failed to load texture");
			return TextureInfo();
		}
		stbi_image_free(data);

		TextureInfo texture_info{};
		texture_info.texture = texture;
		texture_info.height = height;
		texture_info.width = width;
		texture_info.channel_number = nrChannels;

		return texture_info;
	}

	void Renderer::ReCreateFrameBuffers()
	{
		auto& app = Application::Get();
		current_window_height = app.GetWindow().GetHeight();
		current_window_width = app.GetWindow().GetWidth();

		m_WorkGroupsX = (current_window_width + ((int)current_window_width % 16)) / 16;
		m_WorkGroupsY = (current_window_height + ((int)current_window_height % 16)) / 16;

		glGenTextures(1, &m_DepthTexture);
		glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, current_window_width, current_window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glGenFramebuffers(1, &m_DepthFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glGenFramebuffers(1, &m_HDRFBO);

		glGenTextures(1, &m_ColorBuffer);
		glBindTexture(GL_TEXTURE_2D, m_ColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, current_window_width, current_window_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenRenderbuffers(1, &m_RBODepth);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBODepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, current_window_width, current_window_height);

		glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorBuffer, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBODepth);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		size_t numberOfTiles = m_WorkGroupsX * m_WorkGroupsY;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_VisibleLightsBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleIndex) * 1024, 0, GL_STATIC_DRAW);

	}
	void Renderer::UploadLightData()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightsBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_PointLights.size() * sizeof(PointLightInfo), 0, GL_DYNAMIC_DRAW);

		PointLightInfo* pointLights = (PointLightInfo*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

		for (size_t i = 0; i < m_PointLights.size(); i++) {
			PointLightInfo& light = pointLights[i];
			light.color = glm::vec4(m_PointLights[i]->GetColor(), 1.f);
			light.intensity = m_PointLights[i]->GetIntensity();
			light.position = glm::vec4(m_PointLights[i]->GetPosition(), 1.f);
			light.constantAttenuation = m_PointLights[i]->GetConstantAttenuation();
			light.linearAttenuation = m_PointLights[i]->GetLinearAttenuation();
			light.quadraticAttenuation = m_PointLights[i]->GetQuadraticAttenuation();
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	void Renderer::DrawHDRQuad()
	{
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
	bool Renderer::OnWindowResized()
	{
		auto& app = Application::Get();
		float height = app.GetWindow().GetHeight();
		float width = app.GetWindow().GetWidth();
		m_CurrentCamera->SetAspectRatio(width / height);
		glViewport(0, 0, width, height);
		ReCreateFrameBuffers();
		return true;
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