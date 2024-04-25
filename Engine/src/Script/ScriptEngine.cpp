#include "pch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

// Ripped initialization code from: https://nilssondev.com/mono-guide/book/introduction.html. Thanks


namespace Engine {

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

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
	};

	ScriptData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptData();
		InitMono();
		LoadAssembly("Resources/Scripts/ScriptCore.dll");

		LoadAssemblyClasses(s_Data->CoreAssembly);


		ScriptGlue::RegisterFunctions();

		// Create c# object
		/*MonoClass* assemlyclass = mono_class_from_name(s_Data->CoreAssemblyImage, "Engine", "Main");
		MonoObject* instance = mono_object_new(s_Data->AppDomain, assemlyclass);
		mono_runtime_object_init(instance);

		MonoMethod* print_message_func = mono_class_get_method_from_name(assemlyclass, "PrintMessage", 0);

		mono_runtime_invoke(print_message_func, instance, nullptr, nullptr);

		MonoMethod* print_custom_message_func = mono_class_get_method_from_name(assemlyclass, "PrintCustomMessage", 1);

		MonoString* message = mono_string_new(s_Data->AppDomain, "This param is sent from C++");

		void* param = message;

		mono_runtime_invoke(print_custom_message_func, instance, &param, nullptr);*/
	}
	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}


	void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		s_Data->EntityClasses.clear(); // For reloading later

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

			MonoClass* entityClass = mono_class_from_name(image, "Helios", "Entity");

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
			if (isEntity)
			{
				s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppDomain = mono_domain_create_appdomain((char*)"HeliosScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
	}

	std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
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

	ScriptClass::ScriptClass(const std::string& name_space, const std::string& class_name)
	{
		// m_MonoClass = mono_class_from_name(isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
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
	ScriptInstance::ScriptInstance(Ref<ScriptClass> script_class): m_ScriptClass(script_class)
	{
		m_Instance = script_class->Instantiate();
		m_OnCreateMethod = script_class->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = script_class->GetMethod("OnUpdate", 1);
	}
	void ScriptInstance::InvokeOnCreate()
	{
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}
	void ScriptInstance::InvokeOnUpdate(float delta_time)
	{
		void* param = &delta_time;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
	}
}
