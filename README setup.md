# Raspberry Pi Pico + Pimoroni C++ Sample Project <!-- omit in toc -->

This is based on https://github.com/pimoroni/pico-boilerplate

This is some guidance to set up a C++ project to use a [Pico W](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#pico-1-family) and a [Pimoroni Display Pack 2.0](https://shop.pimoroni.com/products/pico-display-pack-2-0) to learn C++. 

You can use this project a starting point for your own C++ projects. 
This README tries to help with setting up the files for all to compile well.

- [Prerequisites](#prerequisites)
- [Components](#components)
- [Preparing your build environment](#preparing-your-build-environment)
- [Pico SDK](#pico-sdk)
- [Pimoroni libraries](#pimoroni-libraries)
- [LittleFS Libraries](#littlefs-libraries)
- [Prepare Visual Studio Code](#prepare-visual-studio-code)
- [Prepare your project](#prepare-your-project)
- [Pick your LICENSE](#pick-your-license)

## Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/) needs to be installed, as it will be the IDE to code.

## Components

Tools you are going to need:

Linux/Mac
```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi build-essential
```
For Windows, also Ninja

[ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) - ARM C/C++ Compiler and tools

CMake

Ninja


## Preparing your build environment

You want to have a parent directory for Pico SDK packages (Pico SDK, Pimoroni, LittleFS) that we will not touch, but use to generate the appropiate modules for our project. Set the `PICO_DEV` environment variable pointing to that directory, this is very important.  

Linux/Mac - In `~/.profile` or equivalent for your shell:
```
export PICO_DEV="/path/to/pico_dev"
```
In Windows
Go to the Start Menu, type `env` and select Edit the System Environment Variables
Click the `Environment Variables...` button
Add the PICO_DEV user environment variable 


## Pico SDK
```
cd /path/to/pico_dev
git clone https://github.com/raspberrypi/pico-sdk
cd pico-sdk
git submodule update --init
```

## Pimoroni libraries
```
cd /path/to/pico_dev
git clone https://github.com/pimoroni/pimoroni-pico
```

## LittleFS Libraries
```
cd /path/to/pico_dev
git clone https://github.com/lurk101/littlefs-lib
```

Your pico_dev directory should look like this now:
```
pico_dev/
├── pico-sdk
├── pimoroni-pico
└── littlefs-lib  
```

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
