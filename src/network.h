#pragma once
#include "engine.h"

void* run_server(void* arg);
void* run_client(void* arg);

void send_player_position(ClientData client_data);
void send_disconnect(ClientData client_data);
