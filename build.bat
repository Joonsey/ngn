@echo off
if not exist build mkdir build
gcc src\static.c -o build\ngn -lraylib -lm -lgdi32 -lwinmm -g src\engine.c
exit /b
