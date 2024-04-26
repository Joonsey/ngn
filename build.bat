@echo off
if not exist build mkdir build
for %f in (src\*.c) do gcc %f -o build\ngn -lraylib -lm -lgdi32 -lwinmm -g
exit /b
