# Terminal based 3D renderer 

A software renderer/rasterizer made in C with no dependencies outside of libc

## Building it locally
### (For linux)
### Pre-requisites
- git
- gcc

Clone the repo

`git clone https://github.com/PGamer2951/tui-software-renderer.git`

Change into the repo's directory

`cd tui-software-renderer`

Compile it with make

`make build`

Run the renderer

`./tui-raster demo.scene`

If instead of rendering to the terminal, you'd prefer an x11 window, simply add `X11` to the end of the command

`./tui-raster demo.scene X11`

## Scene files

Scene files (ending in .scene) can be loaded by the renderer and tell it which objects to load, where to place them and if they have any rotation.

There are five available commands in scene files:

`LOAD <path-to-.obj-file>`

Will load and render a new obj file

It also attributes an id to the object based on the loading order, the first object loaded gets id 0, the second one gets id 1 and so on.

`MOVE <id> <x>/<y>/<z>`

Moves an object with id to a new position of (x, y, z)

`ROTATE <id> <x>/<y>/<z>`

Rotates an object with id to a new rotation of (x, y, z)

`SCALE <id> <x>/<y>/<z>`

Scales up or down an object with id

`ASSIGN <id> <function-name>`

This will assign a new function to the object with id

The function must be declared in main.c and obey the following structure:

```
void foo(Object *obj, Event *ev) {
    ...
}
```

Then, in order to assign this function to an object in your scene, create a NameFunctionPair that pairs the name in your .scene file with your actual function

```
NameFunctionPair funcs[1] = {
    {"foo", foo},
};
```

Then you pass the array of NameFunctionPairs to `LoadSceneFromFile()`, along with the number of custom functions you defined

```
int sceneSize = LoadSceneFromFile(argv[1], &scene, funcs, 1);
```



### Example scene

```
LOAD data/cars/Car4.obj
LOAD data/cars/Car5.obj
LOAD data/cars/Car6.obj

MOVE 0 -8/0/-2
MOVE 1 0/0/-2
MOVE 2 8/0/-2

ROTATE 1 0/90/0

ASSIGN 1 Spin
```

## Useful links

[Learning Modern 3D Graphics Programming](https://paroj.github.io/gltut/)

[Rasterization using Barycentric coordinates](https://ccgomezn.github.io/vc/docs/workshops/rendering/barycentric)
