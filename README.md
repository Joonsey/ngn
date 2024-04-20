# Another Game Engine

Just another rudamentary game engine to experiment with rendering pipelines in OpenGL.

## Quick start

### Pre-requisite

#### Linux

We use [raylib](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux) in the backend **NOTE:** remember to build it so that it can be dynamically linked

```sh
git clone https://github.com/raysan5/raylib.git raylib
cd raylib/src/
make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED # To make the dynamic shared version.
sudo make install RAYLIB_LIBTYPE=SHARED # Dynamic shared version.
```

to compile and run the engine
```sh
./build.sh && build/ngn
```

#### Windows

Probably doesn't work at all. No idea. If you want to try remember that we're building the engine.c into an object file and building a shared library. This is because we want to hot-reload the symbols during run time for debugging. Might remove this and optionally staticaly link if requested.

### Debugging

The `build.sh` file builds with debug symbols so that you can run `./build.sh && gdb build/ngn` if you want to run it in GDB.
This can be nice, also particularly because we often times want to hot-reload the engine while testing. (Keep in mind there are some clear limits to the hot-reload feature)

**NOTE:** If you find it annoying to leave and enter gdb everytime, you can build and source the new binary from within gdb as well
```sh
gdb
```
```gdb
shell ./build.sh && file build/ngn
```
