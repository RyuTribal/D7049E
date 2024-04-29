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

	private:
		Ref<ScriptClass> m_ScriptClass;

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

		static void OnCreateEntityClass(Entity* entity);

		static void OnUpdate(float delta_time);

		static std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
		static std::unordered_map<UUID, Ref<ScriptInstance>>& GetEntityInstances();
		static Scene* GetSceneContext();

		static MonoImage* GetCoreAssemblyImage();

	private:
		static void InitMono();
		static void ShutdownMono();
		static MonoObject* InstantiateClass(MonoClass* mono_class);
		static void LoadAssemblyClasses();

		friend class ScriptClass;
		friend class ScriptGlue;
	};
}
