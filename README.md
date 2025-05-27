# csound.node
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-extended-node/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## License

csound-extended-node is copyright (c) 2021 by Michael Gogins and 
other contributors to the csound-node-extended repository.

csound-extended-node is free software; you can redistribute it
and/or modify them under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

csound-extended-node is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
02110-1301 USA

## Introduction

[NW.js](http://nwjs.io/), which used to be called Node-Webkit, is a system for building 
applications for personal computers and workstations based on Web browser 
technology. Typically the user interface is defined in HTML and the program 
logic is defined in JavaScript. C++ addons, which expose JavaScript functions 
that call into C++ libraries, also can be used. The application executes in an 
embedded Web browser based on Google Chrome.

csound.node is a C++ addon for NW.js that embeds [Csound](http://csound.github.io/) into the 
JavaScript context of Web pages running in NW.js. Such pages can call core 
methods of the Csound API as member functions of a `csound` object that 
belongs to the window.

Therefore, NW.js with `csound.node` can be used not only for composing and 
performing Csound pieces, but also for developing standalone applications that 
incorporate Csound. It can be used, for example, to develop sound art 
installations, visual music, or kiosk-type applications.

The `csound-examples/docs/message.html` piece is an example of an HTML file
that embeds not only Csound, but also a Csound orchestra and score. See below 
for how to run such pieces.

The motivation for csound.node should be obvious. It works on all personal 
computer platforms, the build steps are simple, and the end product makes all 
of the myriad capabilities of HTML5 available to Csound pieces, including 
JavaScript, user-defined HTML user interfaces, 3-dimensional animated computer
graphics, and much more. For a full list of capabilities currently implemented 
in HTML5, see [this HTML5 test page](https://html5test.com/).

In addition, the Csound API in csound.node is compatible with the Csound API 
defined in [csound-wasm](https://github.com/gogins/csound-wasm) and 
used by [cloud-5](https://github.com/gogins/cloud-5). Therefore, many pieces 
can be rendered in either environment, although csound.node does afford 
additional capabilities such as accessing the local filesystem, loading plugin 
opcodes, and running somewhat faster.

Please log any bug reports or requests for enhancements at 
https://github.com/gogins/csound-extended-node/issues.

## Usage

A composition that uses csound.node is a regular NW.js application. This is 
rather different from a typical desktop application. 

Currently the following steps work on macOS, but similar steps would be taken 
on other platforms. The build uses 
[nw-builder](https://github.com/nwutils/nw-builder).

For a sample NW.js composition that you can use as a template for your own 
pieces, see Poustinia-v5c.

1. Create an application directory containing a `src` subdirectory and a 
   `build` subdirectory. 
2. Change to the `src` directory.
3. Create your piece as an .html file.
4. Also in the `src` directory place any scripts, images, or other assets that 
   will be loaded from your piece.
2. Create a `package.json` application manifest similar to this:
```
{
  "name": "MyPiece",
  "main": "MyPiece.html",
  "version": "0.1.0",
  "keywords": [ "Csound", "node-webkit" ],
  "window": {
    "toolbar": false,
    "frame": false,
    "maximized": true,
    "position": "mouse",
    "fullscreen": true
  }
}
```
4. Run `pnpm add nw-builder -D` to install nwbuild.
5. To create a temporary build and play or debug your piece, run 
   `pnpm nwbuild --glob=false --mode=run --version=latest --flavor=sdk .` 
   to both install NW.js (in the `cache`` subdirectory) and run the composition app.
6. To build your piece as a standalone application that can be installed, run
   `pnpm nwbuild --glob=false --mode=build --version=latest --flavor=sdk --outDir=../build .`
7. You can then play your piece at any time with `open ../build/MyPiece.app`.

## Building `csound.node`

csound-extended-node is built in the "npm way" but uses cmake.js rather than 
node-gyp to compile the addon.

1. Make a local clone of this repository.

2. Install Csound from `https://csound.com/download.html`. On Linux, it is 
   easy to build Csound from source code.

3. Install npm.

4. Install node-addon-api: `npm install -g node-addon-api`.

5. Install cmake-js: `npm install -g cmake-js`.

6. The following environment variable MUST be set before using, perhaps in
your .profile script. `NODE_PATH` must include the pathname to the directory 
where csound.node has been built. The example is for macOS, so you must 
change this to suit your environment.

```
NODE_PATH=/usr/local/lib
```

7. CMakeLists.txt is configured for building on macOS. You will need to 
   change the library and include directories configuration to suit your 
   own environment. To actually build execute:
```
cmake-js rebuild
```

## Release Notes

### [v0.3.2](https://github.com/gogins/csound-wasm/commits/v0.3.2)

 - Updated Csound to version 6.19.0.

 - Improved the WebAssembly builds of Csound and CsoundAC to support 
   running either in NW.js with native Csound, or in Web browsers with 
   Csound for WebAssembly. This makes it possible, e.g., to compose pieces 
   using Strudel with native Csound, VST plugins, access to the local 
   filesystem, and so on.

