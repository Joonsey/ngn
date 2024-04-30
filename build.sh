mkdir -p build

gcc src/*.c -o build/ngn -lraylib -lm -g -Wall
