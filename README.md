# tmx2gba #
tmx2gba is a simple command line utility that converts [Tiled](http://www.mapeditor.org/) .tmx maps to Game Boy Advance formatted charmaps.

### Features ###
* Export raw charmaps that can be easily memcpy'd into VRAM.
* Preserves tile flipping.
* Supports per-tile palette specification.
* Custom collision layer support.
* Support for objects with id mapping.

## Usage ##
```
tmx2gba [-hv] [-r offset] [-lyc name] [-p 0-15] [-m name;id] <-i inpath> <-o outpath>
```

| Command      | Required | Notes                                                                              |
|--------------|----------|------------------------------------------------------------------------------------|
| -h           | N/A      | Display help & command info                                                        |
| -v           | No       | Display version & quit                                                             |
| -l (name)    | No       | Name of layer to use (default first layer in TMX)                                  |
| -y (name)    | No       | Layer for palette mappings                                                         |
| -c (name)    | No       | Output a separate 8bit collision map of the specified layer                        |
| -r (offset)  | No       | Offset tile indices (default 0)                                                    |
| -p (0-15)    | No       | Select which palette to use for 4-bit tilesets                                     |
| -m (name;id) | No       | Map an object name to an ID, will enable object exports                            |
| -i (path)    | *Yes*    | Path to input TMX file                                                             |
| -o (path)    | *Yes*    | Path to output files                                                               |
| -f <file>    | No       | Flag file containing command-line arguments for easy integration with buildscripts |

## Building ##

Dependencies for building are CMake 3.15 and a C++20 compliant compiler,
all other dependencies are in-tree so you should be able to build with:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

Optionally, you may install it to use it system wide:
```bash
sudo cmake --install build
```
Which will copy the tmx2gba executable to /usr/local/bin/tmx2gba by default,
`--prefix /usr` can be used to override install location.

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

## License ##
[tmx2gba](https://github.com/ScrelliCopter/tmx2gba) is licensed under the [Zlib license](COPYING.txt).
- A modified [tmxlite](https://github.com/fallahn/tmxlite) is licensed under the [Zlib license](ext/tmxlite/LICENSE).
- [pugixml](https://pugixml.org/) is licensed under the [MIT license](ext/pugixml/LICENSE.md).
- [René Nyffenegger's base64.cpp](https://github.com/ReneNyffenegger/cpp-base64) is licensed under the [Zlib license](ext/base64/LICENSE).
- [miniz](https://github.com/richgel999/miniz) is licensed under the [MIT license](ext/miniz/LICENSE).
- [ZStandard](https://facebook.github.io/zstd/) is licensed under the [BSD 3-clause license](ext/zstd/LICENSE).
