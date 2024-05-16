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

		Ref<Texture2D> Black_Flat_Cube;
	};
	
	static DefaultTextures* s_DefaultTextures;

    Renderer* Renderer::s_Instance = nullptr;

	namespace Utils {
		glm::mat4 GetLightSpaceMatrix(const float nearPlane, const float farPlane, const glm::vec3& lightDir)
		{
			auto camera = Renderer::Get()->GetCamera();
			auto settings = Renderer::Get()->GetSettings();
			const auto corners = camera->GetFrustumCornersWorldSpace();

			glm::vec3 min(INFINITY, INFINITY, INFINITY);
			glm::vec3 max(-INFINITY, -INFINITY, -INFINITY);
			for (const auto& v : corners)
			{
				glm::vec3 v3 = glm::vec3(v);
				min = glm::min(min, v3);
				max = glm::max(max, v3);
			}
			glm::vec3 center = (min + max) / 2.f;

			const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::lowest();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::lowest();
			for (const auto& v : corners)
			{
				const auto trf = lightView * v;
				minX = std::min(minX, trf.x);
				maxX = std::max(maxX, trf.x);
				minY = std::min(minY, trf.y);
				maxY = std::max(maxY, trf.y);
				minZ = std::min(minZ, trf.z);
				maxZ = std::max(maxZ, trf.z);
			}

			constexpr float zMult = 100.0f;
			if (minZ < 0)
			{
				minZ *= zMult;
			}
			else
			{
				minZ /= zMult;
			}
			if (maxZ < 0)
			{
				maxZ /= zMult;
			}
			else
			{
				maxZ *= zMult;
			}

			const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
			return lightProjection * lightView;
		}
	}

    Renderer::Renderer()
    {
		m_RendererAPI.Init();

		s_DefaultTextures = new DefaultTextures();

		TextureSpecification default_texture_spec;
		default_texture_spec.Format = ImageFormat::RGBA8;
		default_texture_spec.Width = 1;
		default_texture_spec.Height = 1;

		TextureSpecification default_cube_texture_spec;
		default_cube_texture_spec.Format = ImageFormat::RGBA16F;
		default_cube_texture_spec.Width = 1;
		default_cube_texture_spec.Height = 1;

		uint32_t whiteTextureData = 0xffffffff;
		s_DefaultTextures->White = Texture2D::Create(default_texture_spec, Buffer(&whiteTextureData, sizeof(uint32_t)));

		uint32_t blackTextureData = 0xff000000;
		s_DefaultTextures->Black = Texture2D::Create(default_texture_spec, Buffer(&blackTextureData, sizeof(uint32_t)));

		s_DefaultTextures->Black_Flat_Cube = Texture2D::Create(default_cube_texture_spec, Buffer(&blackTextureData, sizeof(uint32_t)));

		uint32_t grayTextureData = 0xff808080;
		s_DefaultTextures->Gray = Texture2D::Create(default_texture_spec, Buffer(&grayTextureData, sizeof(uint32_t)));

		uint32_t blueTextureData = 0xffff8080;
		s_DefaultTextures->Blue = Texture2D::Create(default_texture_spec, Buffer(&blueTextureData, sizeof(uint32_t)));

		m_LightsSSBO = CreateRef<ShaderStorageBuffer>(sizeof(PointLightInfo) * MAX_POINT_LIGHTS, 2);
		m_DirLightsSSBO = CreateRef<ShaderStorageBuffer>(sizeof(DirectionalLightInfo) * MAX_DIR_LIGHTS, 0);

		current_window_width = Application::Get().GetWindow().GetWidth();
		current_window_height = Application::Get().GetWindow().GetHeight();

		FramebufferSpecification depthSpec = {};
		depthSpec.Width = current_window_width;
		depthSpec.Height = current_window_height;
		depthSpec.Attachments = { FramebufferTextureFormat::DEPTH24STENCIL8 };
		m_DepthFramebuffer = Framebuffer::Create(depthSpec);

		FramebufferSpecification intermidiateSpec = {};
		intermidiateSpec.Width = current_window_width;
		intermidiateSpec.Height = current_window_height;
		intermidiateSpec.Attachments = {
				FramebufferTextureFormat::RGBA16F,
				FramebufferTextureFormat::DEPTH24STENCIL8
		};
		m_Intermidiatebuffer = Framebuffer::Create(intermidiateSpec);

		FramebufferSpecification sceneSpec = {};
		sceneSpec.Width = current_window_width;
		sceneSpec.Height = current_window_height;
		sceneSpec.Attachments = {
				FramebufferTextureFormat::RGBA16F,
				FramebufferTextureFormat::DEPTH24STENCIL8
		};
		m_SceneFramebuffer = Framebuffer::Create(sceneSpec);

		RecreateDirLightShadowBuffer();

		RecreateBuffers();

		m_ShaderLibrary.Load("default_static_pbr", "Resources/Shaders/default_static_shader");
		m_ShaderLibrary.Load("dir_light_shadows", "Resources/Shaders/dir_light_shadows");
		m_ShaderLibrary.Load("forward_plus_depth_pre_pass", "Resources/Shaders/depth_pre_pass");
		m_ShaderLibrary.Load("forward_plus_light_culling", "Resources/Shaders/light_culling_shader");
		m_ShaderLibrary.Load("hdr_shader", "Resources/Shaders/hdr_shader");
		m_ShaderLibrary.Load("line_shader", "Resources/Shaders/line");
		m_ShaderLibrary.Load("rect_to_cube", "Resources/Shaders/equirectangular_map");
		m_ShaderLibrary.Load("env_prefilter", "Resources/Shaders/env_map_prefilter");
		m_ShaderLibrary.Load("env_brdf", "Resources/Shaders/env_brdf");
		m_ShaderLibrary.Load("env_map_convolution", "Resources/Shaders/env_map_convolution");
		m_ShaderLibrary.Load("skybox_shader", "Resources/Shaders/skybox");

		ResizeBuffers();

		m_LightMatricesBuffer = UniformBuffer::Create(sizeof(glm::mat4x4) * 16, 0);

		CreateSkybox(m_Settings.Skybox);

		auto brdf_shader = m_ShaderLibrary.Get("env_brdf");
		if (brdf_shader)
		{
			FramebufferSpecification brdfSpec = {};
			brdfSpec.Width = 512;
			brdfSpec.Height = 512;
			brdfSpec.Attachments = {
					FramebufferTextureFormat::RG16F,
					FramebufferTextureFormat::DEPTH24STENCIL8
			};
			m_BRDFBuffer = Framebuffer::Create(brdfSpec);

			m_BRDFBuffer->Bind();
			m_RendererAPI.SetViewport(0, 0, 512, 512);
			brdf_shader->Activate();
			m_RendererAPI.ClearAll();
			m_RendererAPI.DrawQuad();
			m_BRDFBuffer->Unbind();
		}
		else
		{
			HVE_CORE_WARN("No brdf shader detected, this will make the pbr shader look weird");
		}

    }

    Renderer::~Renderer() {
		delete s_DefaultTextures;
    }


	void Renderer::CreateSkybox(SkyboxSettings& settings)
	{
		if (settings.Texture == nullptr)
		{
			return;
		}

		FramebufferSpecification skyboxSpec = {};
		skyboxSpec.Width = settings.Texture->GetHeight(); // it returns the size anyway
		skyboxSpec.Height = settings.Texture->GetHeight();
		skyboxSpec.Attachments = {
				FramebufferTextureFormat::RGBA32F,
				FramebufferTextureFormat::DEPTH24STENCIL8
		};
		Ref<Framebuffer> skybox_fb = Framebuffer::Create(skyboxSpec);

		auto rect_to_cube_shader = m_ShaderLibrary.Get("rect_to_cube");

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		rect_to_cube_shader->Set("u_EnvironmentMap", 0);
		rect_to_cube_shader->Set("u_Projection", captureProjection);
		settings.Texture->GetFlatTexture()->Bind();

		m_RendererAPI.SetViewport(0, 0, settings.Texture->GetHeight(), settings.Texture->GetHeight());
		skybox_fb->Bind();
		for (unsigned int i = 0; i < 6; ++i)
		{
			rect_to_cube_shader->Set("u_View", captureViews[i]);
			skybox_fb->SetTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, settings.Texture->GetRendererID());
			m_RendererAPI.ClearAll();

			rect_to_cube_shader->Activate();
			m_RendererAPI.DrawCube();
		}

		skybox_fb->Unbind();


		settings.Texture->GenerateMipMap();


		settings.PrefilterMap = TextureCube::Create(settings.Texture->GetFlatTexture(), settings.PrefilterResolution);
		settings.PrefilterMap->GenerateMipMap();

		auto prefilter_shader = m_ShaderLibrary.Get("env_prefilter");

		prefilter_shader->Set("u_EnvironmentMap", 0);
		prefilter_shader->Set("u_Projection", captureProjection);

		settings.Texture->Bind();
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = static_cast<unsigned int>(settings.PrefilterResolution * std::pow(0.5, mip));
			unsigned int mipHeight = static_cast<unsigned int>(settings.PrefilterResolution * std::pow(0.5, mip));
			skybox_fb->Resize(mipWidth, mipHeight);
			m_RendererAPI.SetViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilter_shader->Set("u_Roughness", roughness);
			skybox_fb->Bind();
			for (unsigned int i = 0; i < 6; ++i)
			{
				prefilter_shader->Set("u_View", captureViews[i]);
				skybox_fb->SetTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, settings.PrefilterMap->GetRendererID(), mip);
				prefilter_shader->Activate();

				m_RendererAPI.ClearAll();
				m_RendererAPI.DrawCube();
			}
			skybox_fb->Unbind();
		}

		skybox_fb->Resize(settings.IrradianceResolution, settings.IrradianceResolution);

		auto irradiance_shader = m_ShaderLibrary.Get("env_map_convolution");
		irradiance_shader->Set("u_EnvironmentMap", 0);
		irradiance_shader->Set("u_Projection", captureProjection);

		settings.IrradianceTexture = TextureCube::Create(settings.Texture->GetFlatTexture(), settings.IrradianceResolution);

		settings.Texture->Bind();

		skybox_fb->Bind();
		m_RendererAPI.SetViewport(0, 0, settings.IrradianceResolution, settings.IrradianceResolution);

		for (unsigned int i = 0; i < 6; ++i)
		{
			irradiance_shader->Set("u_View", captureViews[i]);
			skybox_fb->SetTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, settings.IrradianceTexture->GetRendererID());
			m_RendererAPI.ClearAll();

			irradiance_shader->Activate();
			m_RendererAPI.DrawCube();
		}

		skybox_fb->Unbind();
	}

	void Renderer::SubmitObject(Ref<Mesh> mesh)
	{
		HVE_PROFILE_FUNC();
		if (!mesh->GetMeshSource())
		{
			return;
		}
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

	void Renderer::SubmitDebugCapsule(DebugCapsule capsule)
	{
		m_DebugCapsules.push_back(capsule);
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

	void Renderer::SetAntiAliasing(AntiAliasingSettings& settings)
	{
		m_Settings.AntiAliasing = settings;
		RecreateBuffers();
	}

	void Renderer::SetSkybox(SkyboxSettings& settings)
	{
		if (settings.Texture != m_Settings.Skybox.Texture)
		{
			CreateSkybox(settings);
		}
		m_Settings.Skybox = settings;
	}

	void Renderer::DepthPrePass()
	{
		HVE_PROFILE_FUNC();
		m_DepthFramebuffer->Bind();
		m_RendererAPI.ClearDepth();
		Ref<ShaderProgram> shader = m_ShaderLibrary.Get("forward_plus_depth_pre_pass");
		shader->Set("u_CameraView", m_CurrentCamera->GetView());
		shader->Set("u_CameraProjection", m_CurrentCamera->GetProjection());
		shader->Activate();
		m_RendererAPI.SetViewport(0, 0, current_window_width, current_window_height);
		for (Ref<Mesh> mesh : m_Meshes) {
			DrawIndexed(mesh, false);
		}

		m_DepthFramebuffer->Unbind();


		// Shadow maps

		m_SunShadowBuffer->Bind();
		m_RendererAPI.ClearDepth();

		if (m_DirectionalLights.size() < 1 || !m_DirectionalLights[0]->IsCastingShadows())
		{
			return;
		}

		if (m_Settings.ShadowSettings.LastCameraFarPlane != m_CurrentCamera->GetFar())
		{
			m_Settings.ShadowSettings.LastCameraFarPlane = m_CurrentCamera->GetFar();
			RecreateDirLightShadowBuffer();
		}

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : m_CurrentCamera->GetFrustumCornersWorldSpace())
		{
			center += glm::vec3(v);
		}
		center /= m_CurrentCamera->GetFrustumCornersWorldSpace().size();

		m_Settings.ShadowSettings.DirLightView = glm::lookAt(
										(-1.f)*center + m_DirectionalLights[0]->GetDirection(),
										center,
										glm::vec3(0.0f, 1.0f, 0.0f));

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : m_CurrentCamera->GetFrustumCornersWorldSpace())
		{
			const auto trf = m_Settings.ShadowSettings.DirLightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}


		constexpr float zMult = 100.0f;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}

		m_Settings.ShadowSettings.DirLightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

		std::vector<glm::mat4> lightMatrices;
		glm::vec3 lightDir = -glm::normalize(m_DirectionalLights[0]->GetDirection());
		for (size_t i = 0; i < m_Settings.ShadowSettings.ShadowCascadeLevels.size() + 1; ++i)
		{
			if (i == 0)
			{
				lightMatrices.push_back(Utils::GetLightSpaceMatrix(m_CurrentCamera->GetNear(), m_Settings.ShadowSettings.ShadowCascadeLevels[i], lightDir));
			}
			else if (i < m_Settings.ShadowSettings.ShadowCascadeLevels.size())
			{
				lightMatrices.push_back(Utils::GetLightSpaceMatrix(m_Settings.ShadowSettings.ShadowCascadeLevels[i - 1], m_Settings.ShadowSettings.ShadowCascadeLevels[i], lightDir));
			}
			else
			{
				lightMatrices.push_back(Utils::GetLightSpaceMatrix(m_Settings.ShadowSettings.ShadowCascadeLevels[i - 1], m_CurrentCamera->GetFar(), lightDir));
			}
		}

		for (size_t i = 0; i < m_Settings.ShadowSettings.ShadowCascadeLevels.size(); ++i)
		{
			m_LightMatricesBuffer->SetData(&lightMatrices[i], sizeof(glm::mat4x4), i * sizeof(glm::mat4x4));
		}

		auto dir_shader = m_ShaderLibrary.Get("dir_light_shadows");
		dir_shader->Activate();
		m_RendererAPI.SetViewport(0, 0, m_Settings.ShadowSettings.Resolution, m_Settings.ShadowSettings.Resolution);
		m_RendererAPI.SetCull(CullOption::FRONT);
		for (Ref<Mesh> mesh : m_Meshes)
		{
			DrawIndexed(mesh, false);
		}
		m_RendererAPI.SetCull(CullOption::BACK);
		m_SunShadowBuffer->Unbind();
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
		m_RendererAPI.BindTexture(depthTextureID, 4);
		m_RendererAPI.DispatchCompute(m_WorkGroupsX, m_WorkGroupsY, 1);
	}

	void Renderer::ShadeAllObjects()
	{
		HVE_PROFILE_FUNC();
		m_RendererAPI.SetViewport(0, 0, current_window_width, current_window_height);
		for (size_t i = 0; i < m_Meshes.size(); i++) {
			DrawIndexed(m_Meshes[i], true);
		}
		
	}

	void Renderer::ShadeHDR()
	{
		HVE_PROFILE_FUNC();

		uint32_t color_attachment;
		if (m_Settings.AntiAliasing.Type != AAType::None)
		{
			m_Intermidiatebuffer->CopyFramebufferContent(m_HDRFramebuffer, m_Settings.AntiAliasing.Type == AAType::SSAA ? FramebufferSamplingFormat::Linear : FramebufferSamplingFormat::Nearest); // Needed because the object buffer is a multisampled buffer
			color_attachment = m_Intermidiatebuffer->GetColorAttachmentRendererID();
		}
		else
		{
			color_attachment = m_HDRFramebuffer->GetColorAttachmentRendererID();
		}

		Ref<ShaderProgram> shader = m_ShaderLibrary.Get("hdr_shader");
		shader->Activate();
		m_RendererAPI.BindTexture(color_attachment);
		shader->Set("exposure", exposure);
		shader->Activate();

		m_SceneFramebuffer->Bind();
		m_RendererAPI.ClearAll();
		DrawHDRQuad();
		m_SceneFramebuffer->Unbind();
	}

	void Renderer::DrawSkybox()
	{
		if (m_Settings.Skybox.Texture != nullptr)
		{
			m_RendererAPI.SetDepthFunction(LEqual);
			m_Settings.Skybox.Texture->Bind();
			auto shader = m_ShaderLibrary.Get("skybox_shader");
			glm::mat4 view = glm::mat4(glm::mat3(m_CurrentCamera->GetView()));
			shader->Set("u_CameraView", view);
			shader->Set("u_Brightness", m_Settings.Skybox.Brightness);
			shader->Set("u_CameraProjection", m_CurrentCamera->GetProjection());
			shader->Activate();
			m_RendererAPI.DrawCube();
			m_RendererAPI.SetDepthFunction(Less);
			m_Stats.draw_calls++;
		}
	}

	void Renderer::RecreateDirLightShadowBuffer()
	{
		FramebufferSpecification sunSpec = {};
		sunSpec.Width = m_Settings.ShadowSettings.Resolution;
		sunSpec.Height = m_Settings.ShadowSettings.Resolution;
		sunSpec.ArraySize = m_Settings.ShadowSettings.ShadowCascadeLevels.size() + 1;
		sunSpec.Attachments = {
				FramebufferTextureFormat::DEPTH24STENCIL8
		};

		m_SunShadowBuffer = Framebuffer::Create(sunSpec);
	}

	void Renderer::DrawIndexed(Ref<Mesh> mesh, bool use_material)
	{
		HVE_PROFILE_FUNC();
		for (size_t i = 0; i < mesh->GetMeshSource()->GetSubmeshes().size(); i++)
		{
			Ref<Material> material = mesh->GetMeshSource()->GetMaterials()[mesh->GetMeshSource()->GetSubmeshes()[i].MaterialIndex];
			if (use_material)
			{
				m_Settings.Skybox.IrradianceTexture->Bind(10);
				m_Settings.Skybox.PrefilterMap->Bind(11);
				m_RendererAPI.BindTexture(m_BRDFBuffer->GetColorAttachmentRendererID(), 12);
				/*m_RendererAPI.BindTexture(m_SunShadowBuffer->GetDepthAttachmentID(), 13);
				material->Set("u_CascadeCount", (int)m_Settings.ShadowSettings.ShadowCascadeLevels.size());
				for (size_t i = 0; i < m_Settings.ShadowSettings.ShadowCascadeLevels.size(); ++i)
				{
					material->Set("u_CascadePlaneDistances[" + std::to_string(i) + "]", m_Settings.ShadowSettings.ShadowCascadeLevels[i]);
				}*/
				material->Set("u_CameraFarPlane", m_CurrentCamera->GetFar());
				material->Set("u_SunView", m_Settings.ShadowSettings.DirLightView);
				material->Set("u_SunProjection", m_Settings.ShadowSettings.DirLightProjection);
				material->Set("u_EnvironmentBrightness", m_Settings.Skybox.Brightness);
				material->Set("u_CameraPos", m_CurrentCamera->CalculatePosition());
				material->Set("u_CameraView", m_CurrentCamera->GetView());
				material->Set("u_CameraProjection", m_CurrentCamera->GetProjection());
				material->Set("u_NumDirectionalLights", (int)m_DirectionalLights.size());
				if (!material->GetUniformValue<int>("u_UseNormalMap"))
				{
					glEnable(GL_NORMALIZE);
				}
				material->ApplyMaterial();
			}

			auto current_shader = m_ShaderLibrary.GetByShaderID(m_RendererAPI.GetCurrentShaderProgram());
			current_shader->Set("u_Transform", mesh->GetTransform() * mesh->GetMeshSource()->GetSubmeshes()[i].WorldTransform);
			current_shader->Activate();

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

		m_RendererAPI.SetDepthWriting(false);
		DrawSkybox();
		m_RendererAPI.SetDepthWriting(true);

		ShadeAllObjects();

		m_RendererAPI.SetDepthWriting(false);
		DrawDebugObjects();
		m_RendererAPI.SetDepthWriting(true);

		m_HDRFramebuffer->Unbind();

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
		m_DebugCapsules.clear();
    }


	void Renderer::RecreateBuffers()
	{
		// Currently we only need to recreate the object shading buffer
		FramebufferSpecification hdrSpec = {};
		hdrSpec.Width = current_window_width;
		hdrSpec.Height = current_window_height;
		hdrSpec.Samples = 1;
		if (m_Settings.AntiAliasing.Type == AAType::SSAA)
		{
			hdrSpec.Width *= m_Settings.AntiAliasing.Multiplier;
			hdrSpec.Height *= m_Settings.AntiAliasing.Multiplier;
		}
		else if (m_Settings.AntiAliasing.Type == AAType::MSAA)
		{
			hdrSpec.Samples *= m_Settings.AntiAliasing.Multiplier;
		}
		hdrSpec.Attachments = {
				FramebufferTextureFormat::RGBA16F,
				FramebufferTextureFormat::DEPTH24STENCIL8
		};
		m_HDRFramebuffer = Engine::Framebuffer::Create(hdrSpec);
	}

	void Renderer::ResizeBuffers()
	{
		HVE_PROFILE_FUNC();
		
		m_DepthFramebuffer->Resize(current_window_width, current_window_height);
		m_HDRFramebuffer->Resize(current_window_width, current_window_height);
		m_SceneFramebuffer->Resize(current_window_width, current_window_height);
		m_Intermidiatebuffer->Resize(current_window_width, current_window_height);

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
		m_Stats.draw_calls++;
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

			glm::mat4 scaled_transform = glm::scale(box.Transform, box.Size);
			for (auto& vertex : vertices)
			{
				vertex = scaled_transform * glm::vec4(vertex, 1.f);
			}

			int edges[][2] = {
				{0, 1}, {1, 3}, {3, 2}, {2, 0},
				{4, 5}, {5, 7}, {7, 6}, {6, 4},
				{0, 4}, {1, 5}, {2, 6}, {3, 7}
			};

			for (auto& edge : edges)
			{
				m_DebugLines.push_back(Line{ vertices[edge[0]], vertices[edge[1]], box.Color, glm::mat4(1.0f) });
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

		for (auto& capsule : m_DebugCapsules)
		{
			std::vector<glm::vec3> vertices;


			// Top cap
			for (int i = 0; i <= latitudeDivisions / 2; ++i)
			{
				float lat = glm::pi<float>() * i / latitudeDivisions;

				for (int j = 0; j <= longitudeDivisions; ++j)
				{
					float lon = 2.0f * glm::pi<float>() * j / longitudeDivisions;

					float x = capsule.Radius * sin(lat) * cos(lon);
					float y = capsule.Radius * cos(lat) + capsule.HalfHeight;
					float z = capsule.Radius * sin(lat) * sin(lon);

					glm::vec3 pos = glm::vec3(capsule.Transform * glm::vec4(x, y, z, 1.0f));
					vertices.push_back(pos);
				}
			}

			// Bottom cap
			for (int i = latitudeDivisions / 2; i <= latitudeDivisions; ++i)
			{
				float lat = glm::pi<float>() * i / latitudeDivisions;

				for (int j = 0; j <= longitudeDivisions; ++j)
				{
					float lon = 2.0f * glm::pi<float>() * j / longitudeDivisions;

					float x = capsule.Radius * sin(lat) * cos(lon);
					float y = capsule.Radius * cos(lat) - capsule.HalfHeight;
					float z = capsule.Radius * sin(lat) * sin(lon);

					glm::vec3 pos = glm::vec3(capsule.Transform * glm::vec4(x, y, z, 1.0f));
					vertices.push_back(pos);
				}
			}

			float x = 0;
			float y = -capsule.HalfHeight;
			float z = 0;
			vertices.push_back(capsule.Transform* glm::vec4(x, y, z, 1.0f));

			for (int i = 0; i < latitudeDivisions; ++i)
			{
				for (int j = 0; j < longitudeDivisions; ++j)
				{
					int index1 = i * (longitudeDivisions + 1) + j;
					int index2 = index1 + 1;
					int index3 = index1 + (longitudeDivisions + 1);
					int index4 = index3 + 1;

					if (i != latitudeDivisions - 1)
					{
						m_DebugLines.push_back(Line{ vertices[index1], vertices[index2], capsule.Color, glm::mat4(1.0f) });
						m_DebugLines.push_back(Line{ vertices[index1], vertices[index3], capsule.Color, glm::mat4(1.0f) });
					}

					if (i != 0)
					{
						m_DebugLines.push_back(Line{ vertices[index2], vertices[index4], capsule.Color, glm::mat4(1.0f) });
					}
				}
			}
		}

		Ref<ShaderProgram> shader = m_ShaderLibrary.Get("line_shader");
		shader->Set("u_CameraView", m_CurrentCamera->GetView());
		shader->Set("u_CameraProjection", m_CurrentCamera->GetProjection());
		shader->Activate();
		m_RendererAPI.DrawInstancedLines(m_DebugLines);
		m_Stats.draw_calls++;
	}

	void Renderer::ResizeViewport(int width, int height)
	{
		HVE_PROFILE_FUNC();
		if (width == current_window_width && height == current_window_height)
		{
			return;
		}
		current_window_width = width;
		current_window_height = height ? height : 1;
		m_CurrentCamera->SetAspectRatio(current_window_width / current_window_height);
		m_RendererAPI.SetViewport(0, 0, current_window_width, current_window_height);

		ResizeBuffers();
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
