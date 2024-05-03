Usage
=====

.. _install:

Installation
------------

To use Helios, you need to install the following dependencies:

- `Visual Studio 2022 <https://visualstudio.microsoft.com/downloads/>`_
- "Desktop development with C++" extension
- "Universal Windows Platform development" extension

After installing clone the repository:

.. code-block:: console

   git clone https://github.com/RyuTribal/D7049E --recursive

Go to the root directory of the repository and 
run (**AS AN ADMIN**) the **generate_win_projects.bat** script to generate the 
Visual Studio solution.

Running
-------

Now there are two projects which you can run in the solution:

- **Editor** - The main editor project **Default startup project**

- **Editor Launcher** - A launcher that starts the editor **Fairly untested**


To run the startup project, press either **F5** or **Local windows debugger**
in Visual Studio.