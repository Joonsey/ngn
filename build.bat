@echo off
if not exist build mkdir build
gcc src/*.c -o build\ngn -lraylib -lm -lgdi32 -lwinmm -lpthread -g  -lws2_32
exit /b
