# Use a Linux base image
FROM ubuntu:latest AS builder

# Set environment variables
ENV MINGW_PATH="/usr/local/mingw-w64"
ENV SRC_PATH="/src"
ENV OUTPUT_PATH="/build"

# Install necessary packages for building
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    gcc-mingw-w64 \
    binutils-mingw-w64 \
	curl \
	unzip \
    && rm -rf /var/lib/apt/lists/*


# Clone raylib
RUN curl -L -k -o raylib.zip https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win32_mingw-w64.zip
RUN unzip raylib.zip
RUN mv raylib-5.0_win32_mingw-w64 /raylib

# Set up the build directory
WORKDIR $SRC_PATH

# Copy the source code into the container
COPY . .

RUN mkdir $OUTPUT_PATH

# Build the program using MinGW-w64
RUN i686-w64-mingw32-gcc -o $OUTPUT_PATH/ngn.exe $(find src -type f -name "*.c") -I/raylib/include -L/raylib/lib -lraylib -lm -lgdi32 -lwinmm -lpthread -lws2_32 -static
RUN ls $OUTPUT_PATH

# Specify the entrypoint to keep the container running
ENTRYPOINT ["tail", "-f", "/dev/null"]
