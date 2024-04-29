#pragma once

#if defined(PLATFORM_WINDOWS)

extern Engine::Application* Engine::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	Engine::Log::Init();
	HVE_CORE_WARN("Log Initialized");
	auto app = Engine::CreateApplication(argc, argv);
	app->run();
	delete app;
}

// This code is halal certified

#else
#error This engine currently only supports Windows and Linux :(

#endif
