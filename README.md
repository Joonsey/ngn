# Another Game Engine

Just another rudamentary game engine to experiment with rendering pipelines in OpenGL.

## Quick start

### Options

- run with built-in local server

    `build/ngn --local`
    We can run with a built in server on a seperate thread, this is just quick way of hosting. The game will also share some memory with the server

- join a target server

    `build/ngn --join <ip>`
    To join a server you need to target a valid ip address

- host a standalone server

    `build/ngn --server <ip | optional>`
    To host a standalone server without a game client you can run this command.
    omitting the ip will default to loopback interface, which is likely what you would be running on anyway.

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

Windows build does not statically link raylib, recommend using x64-mingw-gcc, alternatively you can compile a binary using the docker image in .Dockerfile

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

### Logging

At the moment the best way to change logging is to change the LOG_LEVEL in the logging initializer in `src/main.c`

```c

	initialize_logging(NLOG_LEVEL); // log level is lowest level logged, inclusive

```
