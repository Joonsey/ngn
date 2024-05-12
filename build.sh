mkdir -p build

filenames=$(find src -type f -name "*.c")

gcc $filenames -o build/ngn -lraylib -lm -g -Wall
