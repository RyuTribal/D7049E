#pragma once

// Include everything that has to be used by the engine app

#include "Core/Application.h"
#include "Core/Layer.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseButtonCodes.h"
#include "Core/Log.h"
#include "Core/Profiler.h"
#include "Core/Timer.h"
#include "Core/IO.h"


// ------------- Assets ------------------
#include "Assets/AssetTypes.h"
#include "Assets/AssetImporter.h"
#include "Assets/AssetManager.h"
#include "Assets/TextureImporter.h"

// ------------- Renderer ----------------
#include "Renderer/Renderer.h"
#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"
#include "Renderer/Texture.h"

// ------------- Physics -----------------
#include "Physics/PhysicsEngine.h"
#include "Physics/Auxiliary/HEMotionType.h"


// ---------------- Scene ----------------
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"

// ---------------- UI -------------------
#include "UI/FilePicker.h"

// ---------------- Utils ----------------
#include "Math/Math.h"
#include "ImGui/ImGuiLayer.h"
#include "Project/Project.h"

//---------------- Sound ----------------
#include "Sound/SoundEngine.h"
#include "Sound/GlobalSource.h"
#include "Sound/LocalSource.h"

// -----------------------------------------
