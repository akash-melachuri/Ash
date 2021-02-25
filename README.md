### Ash (WIP)

Platforms Supported (There may be bugs):
- Windows
- Linux

Build Dependencies:
- Vulkan SDK
    - glslc
- GLFW (included as submodule)
- glm (included as submodule)
- spdlog (included as submodule)
- Vulkan Memory Allocator (included as submodule)
- assimp (included as submodule)
- entt (Included)

If CMake fails to find Vulkan and you have the Vulkan SDK installed, you may need to add the below lines to the top of CMakeLists.txt:
```
set(Vulkan_LIB "path/to/vulkan-1.lib")
set(Vulkan_INCLUDE_DIR "path/to/vulkan/include")
```
