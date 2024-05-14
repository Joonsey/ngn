#pragma once
#include "raylib.h"
#include "raymath.h"

#ifdef _WIN32
  #include "winimports.h"
#else
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #define int_cast
#endif

#define MAX_CLIENTS 4
#define GREET_MAX_LENGTH 6
#define BUFFER_SIZE 1024
#define PLAYER_NOT_CONNECTED_SYMBOL -1

