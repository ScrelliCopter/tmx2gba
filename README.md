# tmx2gba #
tmx2gba is a simple command line utility that converts [Tiled](http://www.mapeditor.org/) .tmx maps to Game Boy Advance formatted charmaps.
Originally developed for my own personal use, I've thrown it up in case this is of use to anyone else.

If you find a bug, please open an issue.

Enjoy!

### Features ###
* Exports to raw binary that can be easily memcpy'd into VRAM.
* Preserves tile flipping.
* Supports per-tile palette specification.
* Custom collision layer support.

### How do I use it? ###
```
tmx2gba [-h] [-r offset] [-lyc name] [-p 0-15] <-i inpath> <-o outpath>
```

| Command      | Required | Notes                                                                 |
|--------------|----------|-----------------------------------------------------------------------|
| -h           | N/A      | Display help & command info.                                          |
| -l (name)    | No       | Name of layer to use (default first layer in TMX).                    |
| -y (name)    | No       | Layer for palette mappings.                                           |
| -c (name)    | No       | Output a separate 8bit collision map of the specified layer.          |
| -r (offset)  | No       | Offset tile indices (default 0).                                      |
| -p (0-15)    | No       | Select which palette to use for 4-bit tilesets.                       |
| -m (name;id) | No       | Map an object name to an ID, will enable object exports.              |
| -i (path)    | *Yes*    | Path to input TMX file.                                               |
| -o (path)    | *Yes*    | Path to output files.                                                 |
| -f <file>    | No       | Command line instructions list for easy integration with buildscripts |

### How do I build it? ###

Dependencies for building are CMake 3.x and a C++11 compliant compiler,
all other dependencies are in-tree so you should be able to build with:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -C build -j$(nproc --all)
```

Optionally, you may install it to use it system wide:
```bash
sudo cmake --install build
```
Which will copy the tmx2gba executable to /usr/local/bin/tmx2gba by default,
if you prefer to use /usr for some reason you may specify a prefix like so:
```bash
sudo cmake --install build --prefix /usr
```
If you're a devkitPro user and would prefer to keep all your development tools compartmentalised
you may optionally install to the tools directory with the `TMX2GBA_DKP_INSTALL` option (OFF by default).
The build scripts will respect your `DEVKITPRO` environment variable but if not set will install to
`/opt/devkitpro/tools/bin/tmx2gba` directly, the `--prefix` argument has no effect in this mode.
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DTMX2GBA_DKP_INSTALL:BOOL=ON
cmake --build build
sudo cmake --install build
```

### Todo list ###
* Add support for multi-SBB prepared charmaps.
* Check if this works for NDS as well.
* Compression support.
* Support for less common TMX formats.

### License ###
[tmx2gba](https://github.com/ScrelliCopter/tmx2gba) is licensed under the zlib license.
[RapidXML](http://rapidxml.sourceforge.net/) is licensed under the Boost & MIT licenses.
[René Nyffenegger's base64.cpp](https://github.com/ReneNyffenegger/cpp-base64) is licensed under the zlib license.
[miniz](https://github.com/richgel999/miniz) is public domain software.
[ultragetopt](https://github.com/kevinoid/ultragetopt) is licensed under the MIT license.

```
  Copyright (C) 2015-2023 a dinosaur

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  
```
