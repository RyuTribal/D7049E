#pragma once

namespace Engine {
	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);

	private:
		static void InitMono();
		static void ShutdownMono();
	};
}
