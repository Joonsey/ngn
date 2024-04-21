mkdir -p build

gcc src/static.c -o build/ngn -lraylib -lm -g src/engine.c
