@echo off
if not exist build mkdir build
set "filenames="
for /R %%f in (src\*.c) do (
  set "filenames=!filenames! %%f "
)

gcc !filenames! -o build/ngn -lraylib -lm -g -Wall
exit /b
