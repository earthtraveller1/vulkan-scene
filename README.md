# Vulkan Scene

yes, a vulkan scene. very interesting indeed

## Quick Start

Ensure that you have the proper dependencies installed first. Of course, you need the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) installed. If you are on Linux, you will also need the X.Org development files (such as `X11/Xlib.h`) installed. All other dependencies are already included with the project.

Next, to run the program, do the following.

```
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
build/vulkan-scene
```

If this doesn't work, please [open an issue](https://github.com/earthtraveller1/vulkan-scene/issues/new/choose) to let me know.
