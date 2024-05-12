
<h1 align="center">Helios engine</h1>
<p align="center">
  <img src="assets/logo.png" alt="Logo" width="600" height="400">
</p>

[![Documentation Status](https://readthedocs.org/projects/d7049e/badge/?version=latest)](https://d7049e.readthedocs.io/en/latest/?badge=latest)

<a href="https://trello.com/b/3jKnPwDf/d7049e-projekt">Project board</a>

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
  <li><a href="#getting-started">Getting Started</a></li>
  <li><a href="#libraries-used">Libraries Used</a></li>
  <li><a href="#main-idea">Main Idea</a></li>
    <li><a href="#coding-best-practices">Coding Best Practices</a></li>
   <!--<ul>
    <li><a href="#general-practices">General Practices</a></li>
    <li><a href="#backend">Backend</a></li>
    <li><a href="#front-end">Front-end</a></li>
    </ul> -->
  

</details>

## Getting Started

You will need Visual Studio 2022 with the addon called "Desktop development with C++".

After that run the <b>generate_win_projects.bat</b> file and you should see a solution file called <b>Helios.sln</b>. Double click it to open it up.
As soon as the project is open, press F5 or the "Local windows debugger" button in the editor.


## Libraries Used

<ul>
  <li><a href="https://github.com/Dav1dde/glad">OpenGL, using the Glad loader (Rendering)</a></li>
  <li><a href="https://github.com/jrouwe/JoltPhysics">Jolt Physics (Physics)</a></li>
  <li><a href="https://github.com/jarikomppa/soloud">SoLoud (Audio)</a></li>
  <li><a href="https://github.com/glfw/glfw">GLFW (Window management)</a></li>
  <li><a href="https://github.com/assimp/assimp">Assimp (Parsing model files)</a></li>
  <li><a href="https://github.com/g-truc/glm">GLM (Linear algebra)</a></li>
  <li><a href="https://github.com/ocornut/imgui">Dear ImGui (Building UI)</a></li>
  <li><a href="https://github.com/gabime/spdlog">SPDLog (Logging)</a></li>
  <li><a href="https://github.com/nothings/stb">STB (Image Loading)</a></li>
  <li><a href="https://github.com/wolfpld/tracy">Tracy (Profiling)</a></li>
  <li><a href="https://premake.github.io/">Premake (Build tools)</a></li>
</ul>

## Main Idea

We are planning to create a third person JRPG demo. It will feature some block puzzle mechanics as well as (if time allows) fight encounter using turn based gameplay.
We will be updating our editor as we add on more features so that when the game is being developed we can do it with more efficiency.

## Coding Best Practices

Generally the point of this section is to blend in into the codebase as much as possible so try to look everywhere else in the code for inpiration.
Try to write easy to read code that most people can read except when it impacts performance.

Some general tips:
The naming convention of variables is as follows:
For private class variables and static variables we use the microsoft naming conventions (using m_ for private variables and s_ for static variables):
```
class ModelLibrary
	{
	public:
		static ModelLibrary* Get()
		{
			if (!s_Instance)
			{
				s_Instance = new ModelLibrary();
			}

			return s_Instance;
		}

		ModelLibrary() = default;
		~ModelLibrary() = default;

		const aiScene* LoadModel(const std::string& file_path);

	private:
		static ModelLibrary* s_Instance;
		std::unordered_map<std::string, Ref<aiScene>> m_Library;
	};
```
For naming of functions use capital letters for each word in a function (GetVariable for example). Setter and Getters should be named Get[insert variable] and Set[insert variable].
Try to use descriptive function names, prefferably longer if it makes the functionality more clear. Otherwise just follow good C++ practices.
