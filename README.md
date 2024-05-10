# Gamp: Graphics, Audio, Multimedia and Processing Library for C++ and WebAssembly

[Original document location](https://jausoft.com/cgit/gamp.git/about/).

*gamp /gămp/* noun
- A large baggy umbrella.
> said to allude to Mrs. Gamp's umbrella, in Dickens's *Martin Chuzzlewit.*
- often: one that is untidily or loosely tied up 
> if you carry an umbrella use it tightly rolled and never as a gamp
> — S. D. Barney

## Git Repository
This project's canonical repositories is hosted on [Gothel Software](https://jausoft.com/cgit/gamp.git/).

## Goals
*Gamp* intends to replicate [JogAmp](https://jogamp.org/)'s experiences to C++ in a condensed form,
allowing a similar workflow in the native space *and* browser environment via [WebAssembly](https://webassembly.org/).

Ironically this will bring back *runs everywhere* including *the web*.

Initial goal is to reimplement [Graph/GraphUI](https://jausoft.com/blog/tag/graph_type_rendering/)
on mentioned platforms.

*Gamp* uses [jaulib](https://jausoft.com/cgit/jaulib.git/about/) for general purpose 
functionality including linear algebra and geometry.

## Online WebAssembly Examples
* [RedSquare](https://jausoft.com/projects/gamp/redsquare01.html)

## Earlier Work
[gfxbox2](https://jausoft.com/cgit/cs_class/gfxbox2.git/about/) of our CS class
evaluated the C++ to WebAssembly path earlier.

## Status
This project is currently very much unstable and heavily *WIP*.

All APIs, if any, will change - but I thought it is nice to share the
development with whoever might be interested.

## Supported Platforms
- C++20 or better, see [jaulib C++ Minimum Requirements](https://jausoft.com/cgit/jaulib.git/about/README.md#cpp_min_req).
- [SDL2 library](https://www.libsdl.org/) is supported
- [emscripten](https://emscripten.org/) for [WebAssembly](https://webassembly.org/) (*optional*)

### Build Dependencies
- CMake >= 3.21 (2021-07-14)
- C++ compiler
  - gcc >= 11 (C++20), recommended >= 12
  - clang >= 13 (C++20), recommended >= 16
- Optional for `lint` validation
  - clang-tidy >= 16
- Optional for `eclipse` and `vscodium` integration
  - clangd >= 16
  - clang-tools >= 16
  - clang-format >= 16
- Optional
  - libunwind8 >= 1.2.1
  - libcurl4 >= 7.74 (tested, lower may work)
- [jaulib](https://jausoft.com/cgit/jaulib.git/about/) *submodule*
- [SDL2 library](https://www.libsdl.org/) is supported
- [emscripten](https://emscripten.org/) for [WebAssembly](https://webassembly.org/) (*optional*)

#### Install on Debian or Ubuntu

Installing build dependencies for Debian >= 12 and Ubuntu >= 22:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
apt install git
apt install build-essential g++ gcc libc-dev libpthread-stubs0-dev 
apt install clang-16 clang-tidy-16 clangd-16 clang-tools-16 clang-format-16
apt install libunwind8 libunwind-dev
apt install cmake cmake-extras extra-cmake-modules pkg-config
apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
apt install doxygen graphviz
apt install emscripten
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If using optional clang toolchain, 
perhaps change the clang version-suffix of above clang install line to the appropriate version.

After complete clang installation, you might want to setup the latest version as your default.
For Debian you can use this [clang alternatives setup script](https://jausoft.com/cgit/jaulib.git/tree/scripts/setup_clang_alternatives.sh).

### Build Procedure

#### Build preparations

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
git clone --recurse-submodules git://jausoft.com/srv/scm/direct_bt.git
cd direct_bt
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<a name="cmake_presets_optional"></a>

#### CMake Build via Presets
Analog to [jaulib CMake build presets](https://jausoft.com/cgit/jaulib.git/about/README.md#cmake_presets_optional) ...

Following debug presets are defined in `CMakePresets.json`
- `debug`
  - default generator
  - default compiler
  - C++20
  - debug enabled
- `debug-gcc`
  - inherits from `debug`
  - compiler: `gcc`
  - disabled `clang-tidy`
- `debug-clang`
  - inherits from `debug`
  - compiler: `clang`
  - enabled `clang-tidy`
- `release`
  - inherits from `debug`
  - debug disabled
- `release-gcc`
  - compiler: `gcc`
  - disabled `clang-tidy`
- `release-clang`
  - compiler: `clang`
  - enabled `clang-tidy`
- `release-wasm`
  - compiler: `clang / emscripten`
  - disabled `clang-tidy`
  - needs to be run by `emcmake`

Kick-off the workflow by e.g. using preset `release-gcc` to configure, build, test, install and building documentation.
You may skip `install` and `doc` by dropping it from `--target`.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
cmake --preset release-gcc
cmake --build --preset release-gcc --parallel
cmake --build --preset release-gcc --target test install doc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<a name="cmake_presets_hardcoded"></a>

#### CMake Build via Hardcoded Presets
Analog to [jaulib CMake hardcoded presets](https://jausoft.com/cgit/jaulib.git/about/README.md#cmake_presets_hardcoded) ...

Besides above `CMakePresets.json` presets, 
`JaulibSetup.cmake` contains hardcoded presets for *undefined variables* if
- `CMAKE_INSTALL_PREFIX` and `CMAKE_CXX_CLANG_TIDY` cmake variables are unset, or 
- `JAU_CMAKE_ENFORCE_PRESETS` cmake- or environment-variable is set to `TRUE` or `ON`

The hardcoded presets resemble `debug-clang` [presets](README.md#cmake_presets_optional).

Kick-off the workflow to configure, build, test, install and building documentation.
You may skip `install` and `doc` by dropping it from `--target`.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
rm -rf build/default
cmake -B build/default
cmake --build build/default --parallel
cmake --build build/default --target test install doc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The install target of the last command will create the include/ and lib/ directories with a copy of
the headers and library objects respectively in your dist location.

#### CMake Variables
Our cmake configure has a number of options, *cmake-gui* or *ccmake* can show
you all the options. The interesting ones are detailed below:

See [jaulib CMake variables](https://jausoft.com/cgit/jaulib.git/about/README.md#cmake_variables) for details.

<a name="unit_testing"></a>

### Unit Testing
*TBD*

### Cross Build
*TBD*

Also provided is a [cross-build script](https://jausoft.com/cgit/direct_bt.git/tree/scripts/build-cross.sh)
using chroot into a target system using [QEMU User space emulation](https://qemu-project.gitlab.io/qemu/user/main.html)
and [Linux kernel binfmt_misc](https://wiki.debian.org/QemuUserEmulation)
to run on other architectures than the host.

You may use [our pi-gen branch](https://jausoft.com/cgit/pi-gen.git/about/) to produce 
a Raspi-arm64, Raspi-armhf or PC-amd64 target image.


## Build Status
*Will be updated*

## IDE Integration

### Eclipse 
Tested Eclipse 2024-03 (4.31).

IDE integration configuration files are provided for 
- [Eclipse](https://download.eclipse.org/eclipse/downloads/) with extensions
  - [CDT](https://github.com/eclipse-cdt/) or [CDT @ eclipse.org](https://projects.eclipse.org/projects/tools.cdt)
  - [CDT-LSP](https://github.com/eclipse-cdt/cdt-lsp) *recommended*
    - Should work with clang toolchain >= 16
    - Utilizes clangd, clang-tidy and clang-format to support C++20 and above
    - Add to available software site: `https://download.eclipse.org/tools/cdt/releases/cdt-lsp-latest`
    - Install `C/C++ LSP Support` in the `Eclipse CDT LSP Category`
  - `CMake Support`, install `C/C++ CMake Build Support` with ID `org.eclipse.cdt.cmake.feature.group`
    - Usable via via [Hardcoded CMake Presets](README.md#cmake_presets_hardcoded) with `debug-clang`

The [Hardcoded CMake Presets](README.md#cmake_presets_hardcoded) will 
use `build/default` as the default build folder with debug enabled.

Make sure to set the environment variable `CMAKE_BUILD_PARALLEL_LEVEL`
to a suitable high number, best to your CPU core count.
This will enable parallel build with the IDE.

You can import the project to your workspace via `File . Import...` and `Existing Projects into Workspace` menu item.

For Eclipse one might need to adjust some setting in the `.project` and `.cproject` (CDT) 
via Eclipse settings UI, but it should just work out of the box.

Otherwise recreate the Eclipse project by 
- delete `.project` and `.cproject` 
- `File . New . C/C++ Project` and `Empty or Existing CMake Project` while using this project folder.

### VSCodium or VS Code

IDE integration configuration files are provided for 
- [VSCodium](https://vscodium.com/) or [VS Code](https://code.visualstudio.com/) with extensions
  - [vscode-clangd](https://github.com/clangd/vscode-clangd)
  - [twxs.cmake](https://github.com/twxs/vs.language.cmake)
  - [ms-vscode.cmake-tools](https://github.com/microsoft/vscode-cmake-tools)
  - [notskm.clang-tidy](https://github.com/notskm/vscode-clang-tidy)
  - [cschlosser.doxdocgen](https://github.com/cschlosser/doxdocgen)
  - [jerrygoyal.shortcut-menu-bar](https://github.com/GorvGoyl/Shortcut-Menu-Bar-VSCode-Extension)

For VSCodium one might copy the [example root-workspace file](https://jausoft.com/cgit/direct_bt.git/tree/.vscode/direct_bt.code-workspace_example)
to the parent folder of this project (*note the filename change*) and adjust the `path` to your filesystem.
~~~~~~~~~~~~~
cp .vscode/direct_bt.code-workspace_example ../direct_bt.code-workspace
vi ../direct_bt.code-workspace
~~~~~~~~~~~~~
Then you can open it via `File . Open Workspace from File...` menu item.
- All listed extensions are referenced in this workspace file to be installed via the IDE
- The [local settings.json](.vscode/settings.json) has `clang-tidy` enabled
  - If using `clang-tidy` is too slow, just remove it from the settings file.
  - `clangd` will still contain a good portion of `clang-tidy` checks


## Support & Sponsorship
*Gamp* is lead by [Gothel Software](https://jausoft.com/).

If you like to utilize *Gamp* in a commercial setting, 
please contact [Gothel Software](https://jausoft.com/) to setup a potential support contract
or *just* help to fund the ongoing effort.

## Common issues
*Bugtracker etc to be setup at a later stage*.

