#include "pch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/mono-debug.h>
#include "Scene/Entity.h"
#include "Scene/Components.h"
#include "Project/Project.h"
#include "FileWatch.hpp"

// Ripped initialization code from: https://nilssondev.com/mono-guide/book/introduction.html. Thanks


namespace Engine {

#ifdef PLATFORM_WINDOWS
	using WatcherString = std::wstring;
#else
	using WatcherString = std::string;
#endif

	namespace Utils {

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				HVE_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		//TODO: move to the asset manager or a file system class
		char* ReadBytes(const std::string& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;
		}


		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath.string(), &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			std::string string = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, string.data(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			delete[] fileData;

			return assembly;
		}
	}

	struct ScriptData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;
		std::filesystem::path CoreAssemblyPath = "Resources/Scripts/ScriptCore.dll";

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;
		std::filesystem::path AppAssemblyPath;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		Scene* SceneContext = nullptr;
		bool ShouldReload = false;

		std::unique_ptr<filewatch::FileWatch<WatcherString>> WatcherHandle = nullptr;
	};

	ScriptData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptData();
		InitMono();
		LoadAssembly(s_Data->CoreAssemblyPath);

		ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();

		s_Data->EntityClass = ScriptClass("Helios", "Entity", true);
	}
	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}


	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear(); // For reloading later

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, name);
			MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Helios", "Entity");

			std::string fullName;
			if (strlen(nameSpace) != 0)
			{
				fullName = fmt::format("{}.{}", nameSpace, name);
			}
			else
			{
				fullName = name;
			}

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isEntity)
			{
				continue;
			}

			s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);

			HVE_CORE_WARN("{} fields", name);
			void* iterator = nullptr; // Mono why?
			//while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator)) // it returns a nullptr when it doesnt have any more fields
			//{
			//	const char* fieldName = mono_field_get_name(field);
			//	MonoType* fieldType = mono_field_get_type(field);

			//	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_field(monoClass, field);

			//	if (attrInfo)
			//	{
			//		for (int i = 0; i < attrInfo->num_attrs; ++i)
			//		{
			//			MonoObject* attr = mono_custom_attrs_get_attr(attrInfo, monoClass);
			//			MonoClass* attrClass = mono_object_get_class(attr);
			//			const char* attrName = mono_class_get_name(attrClass);

			//			if (strcmp(attrName, "HVEEditableField") == 0)
			//			{
			//				HVE_CORE_WARN("    {}", fieldName);

			//			}
			//		}

			//		mono_custom_attrs_free(attrInfo);
			//	}
			//}
		}
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppDomain = mono_domain_create_appdomain((char*)"HeliosScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
	}

	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath);
		if (s_Data->AppAssembly)
		{
			s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
			LoadAssemblyClasses();
			s_Data->WatcherHandle = std::make_unique<filewatch::FileWatch<WatcherString>>(filepath, [](const auto& file, filewatch::Event eventType)
			{
				s_Data->ShouldReload = true;
			});
		}
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->EntityInstances.clear();
	}

	bool ScriptEngine::EntityClassExists(const std::string& full_class_name)
	{
		return s_Data->EntityClasses.find(full_class_name) != s_Data->EntityClasses.end();
	}

	bool ScriptEngine::EntityInstanceExists(const UUID& entity_id)
	{
		return s_Data->EntityInstances.find(entity_id) != s_Data->EntityInstances.end();
	}

	void ScriptEngine::OnCreateEntityClass(Entity* entity)
	{
		const auto script_component = entity->GetComponent<ScriptComponent>();
		if (script_component && EntityClassExists(script_component->Name))
		{
			s_Data->EntityInstances[entity->GetID()]
				= CreateRef<ScriptInstance>(s_Data->EntityClasses[script_component->Name], entity);
			s_Data->EntityInstances[entity->GetID()]->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdate(float delta_time)
	{
		HVE_PROFILE_FUNC();
		for(auto [entity_id, instance] : s_Data->EntityInstances){
			instance->InvokeOnUpdate(delta_time);
		}
	}

	bool ScriptEngine::ShouldReload()
	{
		return s_Data->ShouldReload;
	}

	void ScriptEngine::MarkForReload()
	{
		s_Data->ShouldReload = true;
	}

	std::unordered_map<UUID, Ref<ScriptInstance>>& ScriptEngine::GetEntityInstances()
	{
		return s_Data->EntityInstances;
	}

	std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	Ref<ScriptInstance> ScriptEngine::GetInstanceByEntityID(UUID entity_id)
	{
		auto iter = s_Data->EntityInstances.find(entity_id);
		if (iter == s_Data->EntityInstances.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	void ScriptEngine::CallMethod(MonoObject* monoObject, MonoMethod* managedMethod, const void** parameters)
	{
		HVE_PROFILE_FUNC();

		MonoObject* exception = NULL;
		mono_runtime_invoke(managedMethod, monoObject, const_cast<void**>(parameters), &exception);
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("HeliosJITRuntime");

		HVE_CORE_ASSERT(rootDomain, "No root domain found!");

		s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;
		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}
	MonoObject* ScriptEngine::InstantiateClass(MonoClass* mono_class)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, mono_class);
		mono_runtime_object_init(instance);
		return instance;
	}


	/// ScriptClass ///////////////////////////////////

	ScriptClass::ScriptClass(const std::string& name_space, const std::string& class_name, bool is_core)
	{
		m_MonoClass = mono_class_from_name(is_core ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, name_space.c_str(), class_name.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}
	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}
	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	void ScriptEngine::ReloadAssembly(const std::filesystem::path& app_assembly_path)
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);

		s_Data->AppAssemblyPath = app_assembly_path;

		LoadAssembly(s_Data->CoreAssemblyPath);
		LoadAppAssembly(s_Data->AppAssemblyPath);
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		s_Data->EntityClass = ScriptClass("Helios", "Entity", true);

		s_Data->ShouldReload = false;

		HVE_CORE_WARN("Reloaded Scripts");
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> script_class, Entity* entity): m_ScriptClass(script_class), m_EntityID(entity->GetID())
	{
		m_Instance = script_class->Instantiate();
		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = script_class->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = script_class->GetMethod("OnUpdate", 1);

		// Call Entity constructor
		void* param = &m_EntityID;
		m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		
	}
	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
		{
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
		}
	}
	void ScriptInstance::InvokeOnUpdate(float delta_time)
	{
		if (m_OnUpdateMethod)
		{
			void* param = &delta_time;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}
}
