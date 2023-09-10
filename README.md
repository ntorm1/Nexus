# Nexus
Algoirthmic trading IDE and framework
![alt text](https://github.com/ntorm1/Nexus/blob/master/Nexus.png)

### Features
- Multi window based UI that allows for the devolpment of Algorithmic trading strategies. 
- Allows for multipule strategies to be executed at the same time using different frequencies, logic, portfolios, etc...
- Create abstract strategies using the node editor to adjust strategies and observe changes without having to recompile.
- Generate code for an abstract strategy and compile it into a seperate dll which can be linked at run time to allow for customized and more complex strategies.

### Build
- Notes: only supported setup is Windows with MSVC compiler, supporting c++20, using Visual Studio build system. Dependancies are manged using vcpkg.
- To build, build the nodeeditor dll by opening nodeeditor\CMakeLists.txt in visual studio and building. Then you can open the Nexus Visual Studio solution. You may have to update the solution properties for include or lib directories, the default vcpkg dir is at C:\dev\vcpkg\installed\x64-windows\bin

### Dependencies 
There are a lot, some are nessecary like Qt and AgisCore, others like One Intel TBB could be removed with some modifications to the code, some of which I have provided but working around this would not be recomended.
- One Intel API and Thread Building Blocks, used for parralell execution
- Qt
- (vcpkg) Apache Arrow C++ (for data io)
- (vcpkg) HDF5 C++ (for data io)
- (vcpkg) Eigen3 (linear algebra)
- (vcpkg) pybind11 (for python wrapper)
- (git submodule) Qt advance docking system  (for main docking ui)
- (git submodule) nodeeditor (strategy nodeeditor )
- (git submodule) AgisCore (backtesting framework)