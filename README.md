# tmx2gba #
tmx2gba is a simple command line utility that converts Tiled (http://www.mapeditor.org/) .tmx maps to Game Boy Advance compatible charmaps.
Originally developed for my own personal use, I've thrown it up on glorious Github in case this is of use to anyone else.

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

Command     | Required | Notes
------------|----------|-------------------------------------------------------------
-h          | N/A      | Display help & command info.
-l (name)   | No       | Name of layer to use (default first layer in TMX).
-y (name)   | No       | Layer for palette mappings.
-c (name)   | No       | Output a separate 8bit collision map of the specified layer.
-r (offset) | No       | Offset tile indices (default 0).
-p (0-15)   | No       | Select which palette to use for 4-bit tilesets.
-i (path)   | *Yes*    | Path to input TMX file.
-o (path)   | *Yes*    | Path to output files.

### Todo list ###
* Add support for multi-SBB prepared charmaps.
* Test on & write Makefile for Linux.
* Export to C/ASM with width/height info.
* Check if this works for NDS as well.
* Implement some kind of grit-style parameters file for easy integration into buildscripts.
* Revamp command line arguments.
* Compression support.
* Refactor & Fix bugs.

### License ###
tmx2gba is licensed under the zlib license.
RapidXML is licensed under the Boost & MIT licenses.
René Nyffenegger's base64.cpp is licensed under the zlib license.
XGetopt & miniz are both public domain software.

```
  Copyright (C) 2015 Nicholas Curtis

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
