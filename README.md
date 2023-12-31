# Nexus
Algoirthmic trading IDE and framework
![alt text](https://github.com/ntorm1/Nexus/blob/master/github/Nexus.png)

### Features
- Multi window based UI that allows for the devolpment of Algorithmic trading strategies. 
- Allows for multipule strategies to be executed at the same time using different frequencies, logic, portfolios, etc...
- Create abstract strategies using the node editor to adjust strategies and observe changes without having to recompile.
- Generate code for an abstract strategy and compile it into a seperate dll which can be linked at run time to allow for customized and more complex strategies.

### Build
- Notes: only supported setup is Windows with MSVC compiler, supporting c++20, using Visual Studio build system. Dependancies are manged using vcpkg.
- To build you may have to update the solution properties for include or lib directories, the default vcpkg dir is at C:\dev\vcpkg\installed\x64-windows\bin
Ex:
- run: git clone https://github.com/ntorm1/Nexus
- run: cd Nexus
- run: git submodule update --init --recursive
- Open nodeeditor/CMakeLists.txt in Visual Studio, build and copy QtNodes.dll to x64/debug and release folders of the Nexus Solution.
- cd to external/QScintilla/src. Open x64 Native Tools Command Prompt
- run qmake
- run nmake /f Makefile.Debug (and release), then copy dll files to x64/debug and release
- Open Nexus.sln and build.
  
To get AgisCoreTest running you will need [Google Test](https://learn.microsoft.com/en-us/visualstudio/test/how-to-use-google-test-for-cpp?view=vs-2022)


### Scripting 
- AgisCore uses LuaJIT for scripting allowing for easy creation of strategies with almost no overhead. You can create abstract strategy trees using the 
AgisCore Lua API which evaluates to a C++ strategy tree at run time with comparable time to abstract strategies made using the node editor.
- To use Lua you need to download LuaJIT, see below, and add the Lua header files and lib files to the Visual Studio Nexus project settings. Make sure the
Lua DLL is available in the system path or is copied/output to the Nexus x64 build folder.


### Dependencies 
There are a lot, some are nessecary like Qt and AgisCore, others like One Intel TBB could be removed with some modifications to the code, some of which I have provided but working around this would not be recomended.
- One Intel API and Thread Building Blocks, used for parralell execution
- Qt
- (vcpkg) Apache Arrow C++ (for data io)
- (vcpkg) rapidjson (serialization)
- (vcpkg) HDF5 C++ (for data io)
- (vcpkg) Eigen3 (linear algebra)
- (vcpkg) pybind11 (for python wrapper)
- (git submodule) Qt advance docking system  (for main docking ui)
- (git submodule) nodeeditor (strategy nodeeditor )
- (git submodule) AgisCore (backtesting framework)
- (git submdoule) QScintilla (text editor)
- LuaJIT see: https://luajit.org/download.html (optional lua scripting)

#### Performance
![alt text](https://github.com/ntorm1/Nexus/blob/master/github/perf.png)


#### Codegen 
![alt text](https://github.com/ntorm1/Nexus/blob/master/github/codegen.png)

#### Python Support
![alt text](https://github.com/ntorm1/Nexus/blob/master/github/pybind11.png)
