#include "pch.h"
#include "ScriptEngine.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>


namespace Engine {

	struct ScriptData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	ScriptData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptData();
		InitMono();
	}
	void ScriptEngine::Shutdown()
	{
		delete s_Data;
		ShutdownMono();
	}

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



	MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			// Log some error message using the errorMessage data
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		// Don't forget to free the file data
		delete[] fileData;

		return assembly;
	}


	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("MyScriptRuntime");

		HVE_CORE_ASSERT(rootDomain, "No root domain found!");

		s_Data->RootDomain = rootDomain;
		s_Data->AppDomain = mono_domain_create_appdomain((char*)"HeliosScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/ScriptCore.dll");
		PrintAssemblyTypes(s_Data->CoreAssembly);

		// Testing
		MonoImage* image = mono_assembly_get_image(s_Data->CoreAssembly);


		// Create c# object
		MonoClass* assemlyclass = mono_class_from_name(image, "Engine", "Main");
		MonoObject* instance = mono_object_new(s_Data->AppDomain, assemlyclass);

		mono_runtime_object_init(instance);

		MonoMethod* print_message_func = mono_class_get_method_from_name(assemlyclass, "PrintMessage", 0);

		mono_runtime_invoke(print_message_func, instance, nullptr, nullptr);

		MonoMethod* print_custom_message_func = mono_class_get_method_from_name(assemlyclass, "PrintCustomMessage", 1);
		
		MonoString* message = mono_string_new(s_Data->AppDomain, "This param is sent from C++");

		void* param = message;

		mono_runtime_invoke(print_custom_message_func, instance, &param, nullptr);

		HVE_CORE_ASSERT(false);
	}

	void ScriptEngine::ShutdownMono()
	{

	}
}
