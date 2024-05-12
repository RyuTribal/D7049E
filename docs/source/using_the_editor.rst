Using The Editor
=============

When starting up the Editor you will start with an empty
sandbox project. You can create a new project by clicking
**File -> New Project** or **Ctrl + P**.

After creating a new project, you will see
a new Visual Studio solution file inside
the **ScriptProject** folder. This is your
C# scripting project. Use this for creating scripts.


Some basic concepts
-------------------

- **Entity**: An entity is an object in the scene. Can be either created by right clicking on the scene graph and selecting **Create Entity** or by dragging a mesh into the viewport.
- **Assets**: Assets have to be registered in order to be used by the engine. Uncheck the **Show registered assets only** checkbox in the asset browser to see all assets. To register an asset, double click on it.
- **Components**: Components are the building blocks of an entity. Can be added by clicking on the "+" icon in the properties tab and selecting a component to add.
- **Scripts**: Scripts are written in C# and can be attached to entities. To create a new script, create in in visual studio using the solution file in the **ScriptProject** folder. After creating a script and building the script project, you can attach the new entity class to the entity by creating a script component and selecting it by pressing the "Select class" button. **Note**: If the class doesn't show up in the list, try clicking the "Project -> Reload Scripts" button in the menu.

