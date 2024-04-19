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
#include "Assets/ModelLibrary.h"
#include "Assets/AssetTypes.h"

// ------------- Renderer ----------------
#include "Renderer/Renderer.h"
#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"
#include "Renderer/Texture.h"


// ---------------- Scene ----------------
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"

// ---------------- Serializers ----------
#include "Serialization/ProjectSerializer.h"
#include "Serialization/SceneSerializer.h"

// ---------------- UI -------------------
#include "UI/FilePicker.h"

// ---------------- Utils ----------------
#include "Math/Math.h"
#include "ImGui/ImGuiLayer.h"


// -----------------------------------------
