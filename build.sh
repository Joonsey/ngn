ENGINE_LIB=enginelib.so

mkdir -p build

gcc -c -shared -fpic -Wall -Werror src/engine.c -o build/engine.o -g
gcc -shared -o build/$ENGINE_LIB build/engine.o -lraylib -lm src/room.c -g

gcc src/main.c -o build/ngn -lraylib -lm -g
