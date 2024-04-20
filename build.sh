ENGINE_LIB=enginelib.so

mkdir -p build

gcc -c -shared -fpic -Wall -Werror src/engine.c -o build/engine.o
gcc -shared -o build/$ENGINE_LIB build/engine.o -lraylib -lm

gcc src/main.c -o build/ngn -lraylib -lm -g
