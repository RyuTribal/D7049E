Engine 
======
The engine is responsible for not only doing game logic
but also creating the application framework for which
systems around the engine can be built in, such as the Editor. 

In order to understand the concepts of the engine, the easiest way is to
start with the main entry point of the engine, which is the :ref:`entrypoint` class.

.. _entrypoint:

EntryPoint
----------

The `EntryPoint` class is the main entry point for the engine. It is responsible for running the 
`CreateApplication` function which the user has defined (refer to the `Editor.cpp` file for an example).
It then start the game loop of the :ref:`application` class. 

.. _application:

Application
-----------

The `Application` class is the class that is supposed to be overwritten by the user to handle application and game logic.
The class will call on various functions of it's layers, such as `OnAttach`, `OnUpdate`, `OnDetach`, `OnEvent`, and `OnImGuiRender`.
The `OnUpdate` function is the main game loop of the application, and is called every frame.
From there it is up to the creator of the application to define what happens in the game loop.
Some useful classes/folders to consider are:

- :ref:`renderer`
- :ref:`physicsengine`
- :ref:`audioengine`
- :ref:`input`
- :ref:`events`
- :ref:`scene`

**NOTE**: In the following sections please refer to the code in the **Editor** folder
for examples on how to use the engine.

.. _renderer:

Renderer
--------

The renderer is responsible for rendering the game to the screen. It is built on top of the `OpenGL` API.
Please render to the main rendering header file **Renderer.h** for all rendering functions as well as 
the root folder of the Renderer for other rendering classes.

.. _physicsengine:

Physics Engine
--------------
The physics engine is responsible for handling physics in the game. 
It is built on top of the `JoltPhysics` physics engine.
Please refer to the **PhysicsEngine.h** header file for all physics functions.

.. _audioengine:

Audio Engine
------------

The audio engine is responsible for handling audio in the game.
It is built on top of the `SoLoud` audio engine.

.. _input:

Input
-----

The input system is responsible for handling input in the game.
Check the virtual class `Input.h` in the `Core` folder for all input functions.

.. _events:

Events
------
Handles both subscriptions and dispatching of events as well as all the different 
types of events that can be dispatched.
Check the `Events` folder for all event classes.

.. _scene:

Scene
-----
The scene is responsible for handling the game world. 
It also handles dispatching different components to different parts of the Engine such as.
Please refer to the `Scene` folder for all scene classes as well as the ECS system.
