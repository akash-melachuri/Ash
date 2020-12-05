### Ash (WIP)

Build Dependencies:
- Vulkan SDK
- GLFW (included as submodule)
- glm (included as submodule)
- spdlog (included as submodule)

If CMake fails to find Vulkan and you have the Vulkan SDK installed, you may need to add the below lines to the top of CMakeLists.txt:
```
set(Vulkan_LIB "path/to/vulkan-1.lib")
set(Vulkan_INCLUDE_DIR "path/to/vulkan/include")
```

