#pragma once

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();
	HVE_CORE_WARN("Log Initialized");
	auto app = Engine::CreateApplication();
	app->run();
	delete app;
}


#else
#error This engine currently only supports Windows and Linux :(

#endif
