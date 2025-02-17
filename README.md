# Raspberry Pi Pico + Pimoroni C++ Sample Project <!-- omit in toc -->

This project is based on https://github.com/pimoroni/pico-boilerplate

This is a small project to use a [Pico W](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#pico-1-family) and a [Pimoroni Display Pack 2.0](https://shop.pimoroni.com/products/pico-display-pack-2-0) to learn C++. 

You can use it as a starting point for your own C++ projects.

- [Prerequisites](#prerequisites)
- [Preparing your build environment](#preparing-your-build-environment)
- [Grab the Pimoroni libraries](#grab-the-pimoroni-libraries)
- [Clone this boilerplate](#clone-this-boilerplate)
- [Prepare Visual Studio Code](#prepare-visual-studio-code)
- [Prepare your project](#prepare-your-project)
- [Pick your LICENSE](#pick-your-license)

## Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/) need to be installed, as it will be the IDE to code.


It's easier if you make a `pico` directory or similar in which you keep the SDK, Pimoroni Libraries and your projects alongside each other. This makes it easier to include libraries.

## Preparing your build environment

Install build requirements:

```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi build-essential
```

And the Pico SDK:

```
git clone https://github.com/raspberrypi/pico-sdk
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=`pwd`
cd ../
```

The `PICO_SDK_PATH` set above will only last the duration of your session.

You should should ensure your `PICO_SDK_PATH` environment variable is set by `~/.profile`:

```
export PICO_SDK_PATH="/path/to/pico-sdk"
```

## Grab the Pimoroni libraries

```
git clone https://github.com/pimoroni/pimoroni-pico
```

## Clone this boilerplate

```
git clone https://github.com/pimoroni/pico-boilerplate
cd pico-boilerplate
```

If you have not or don't want to set `PICO_SDK_PATH` you can edit `.vscode/settings.json` to pass the path directly to CMake.

## Prepare Visual Studio Code

Open VS Code and hit `Ctrl+Shift+P`.

Type `Install` and select `Extensions: Install Extensions`.

Make sure you install:

1. C/C++
2. CMake
3. CMake Tools
4. Cortex-Debug (optional: for debugging via a Picoprobe or Pi GPIO)
5. Markdown All in One (recommended: for preparing your own README.md)

## Prepare your project

Edit `CMakeLists.txt` and follow the instructions, you should make sure you:

1. edit your project name
2. include the libraries you need
2. link the libraries to your project

## Pick your LICENSE

We've included a copy of BSD 3-Clause License to match that used in Raspberry Pi's Pico SDK and Pico Examples. You should review this and check it's appropriate for your project before publishing your code.
