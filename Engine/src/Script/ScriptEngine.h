#pragma once

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoString MonoString;
}

namespace Engine {

	class Scene;
	class Entity;

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& name_space, const std::string& class_name, bool is_core = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
	};


	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> script_class, Entity* entity);

		void InvokeOnCreate();
		void InvokeOnUpdate(float delta_time);

		MonoObject* GetMonoObject() { return m_Instance; }
		Ref<ScriptClass> GetClass() { return m_ScriptClass; }

	private:
		Ref<ScriptClass> m_ScriptClass;
		UUID m_EntityID;

		MonoObject* m_Instance = nullptr;

		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadAppAssembly(const std::filesystem::path& filepath);

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& full_class_name);
		static bool EntityInstanceExists(const UUID& entity_id);

		static void ReloadAssembly(const std::filesystem::path& app_assembly_path);

		static bool ShouldReload();
		static void MarkForReload();

		static void OnCreateEntityClass(Entity* entity);

		static void OnUpdate(float delta_time);

		static std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
		static std::unordered_map<UUID, Ref<ScriptInstance>>& GetEntityInstances();
		static Scene* GetSceneContext();

		static MonoImage* GetCoreAssemblyImage();

		template<typename... TArgs>
		static void CallMethodOnAll(const std::string& methodName, TArgs&&... args)
		{
			HVE_PROFILE_SCOPE_DYNAMIC(methodName.c_str());

			auto& instances = GetEntityInstances();

			for (auto [entity_id, instance] : instances)
			{
				CallMethod(entity_id, methodName, std::forward<TArgs>(args)...);
			}
		}


		

		template<typename... TArgs>
		static void CallMethod(UUID entity_id, const std::string& methodName, TArgs&&... args)
		{
			HVE_PROFILE_SCOPE_DYNAMIC(methodName.c_str());

			auto script_instance = GetInstanceByEntityID(entity_id);

			if (!script_instance)
			{
				HVE_CORE_WARN_TAG("ScriptEngine", "Attempting to call method {0} on an invalid instance!", methodName);
				return;
			}

			MonoObject* managedObject = script_instance->GetMonoObject();

			if (managedObject == nullptr)
			{
				HVE_CORE_WARN_TAG("ScriptEngine", "Attempting to call method {0} on an invalid instance!", methodName);
				return;
			}

			constexpr size_t argsCount = sizeof...(args);

			auto script_class = script_instance->GetClass();

			MonoMethod* method = script_class->GetMethod(methodName, argsCount);
			if (method == nullptr)
			{
				HVE_CORE_ERROR_TAG("ScriptEngine", "Failed to find a C# method called {0} with {1} parameters", methodName, argsCount);
				return;
			}

			if constexpr (argsCount > 0)
			{
				const void* data[] = { &args... };
				CallMethod(managedObject, method, data);
			}
			else
			{
				CallMethod(managedObject, method, nullptr);
			}
		}


	private:
		static void InitMono();
		static void ShutdownMono();
		static MonoObject* InstantiateClass(MonoClass* mono_class);
		static void LoadAssemblyClasses();
		static Ref<ScriptInstance> GetInstanceByEntityID(UUID entity_id);
		static void CallMethod(MonoObject* monoObject, MonoMethod* managedMethod, const void** parameters);

		friend class ScriptClass;
		friend class ScriptGlue;
	};
}
