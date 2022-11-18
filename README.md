# Basic Vulkan Scene

A basic Vulkan scene written in pure C and uses a custom build system. Currently, it's not done yet.

## How to Build

I'm too lazy to release prebuilt binaries, so for now, here are the instructions for compiling this application on certain platforms.

### Windows

Ensure that you have the following installed:

- Microsoft Visual Studio (or MSVC command line utilties)
- CMake (for building Vulkan-Loader. Needs to be added to PATH)
- Ninja (for the actual build process, and needs to be added to PATH)
- Python (>=3.6 should be fine, but for best reassurance get the latest version available to you. Does not need to be added to PATH, but would make your life much easier if you do, so I'm gonna assume that you have added Python to PATH).

Then, simply run `python3 build.py` to compile the whole project. The Python scripts will automatically compile and configure any dependencies required and locate the needed toolchains, though they aren't guaranteed to work. If you encounter any problems, please open an issue.

### Linux

Ensure that you have the following installed:

- GCC (You can use Clang if you want, but that will require editing the Python scripts and I'm not gonna get into that for now).
- CMake (for building Vulkan-Loader)
- Ninja (for the actual build process)
- Python (>=3.6 should be fine, but for best reassurance get the latest version available to you.)
- XCB development files (is called `libxcb-dev` on Ubuntu-based systems and can be installed through `apt` or `apt-get`. Not sure what it's called on other distributions, but it should sould something like that.)

I'm gonna assume that all of those (except the XCB development files) are added to PATH.

Now, simply run `python3 build.py` to compile the whole project. The Python scripts will automatically compile and configure any dependencies required and locate the needed toolchains, though they aren't guaranteed to work. If you encounter any problems, please open an issue.

### macOS

As of November 2022, Apple does not support the Vulkan API on any of their systems. This project (as the name suggests) revolves around the Vulkan API. As a result, this project does not support macOS or any Apple-based platforms. We are sorry for any inconvenience.

Now, if you managed to compile and run this project using MoltenVK, open a pull request. I'm too lazy to figure out configuring MoltenVK myself.