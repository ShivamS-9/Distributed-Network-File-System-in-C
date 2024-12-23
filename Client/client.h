#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../packet.h"

#define NAMING_SERVER_PORT 8000 // Default Port Number for Naming Server

// Global Variables
extern char server_ip_address[16]; // IP Address of Naming Server
extern int naming_server_file_descriptor, storage_server_file_descriptor;
extern struct sockaddr_in nm_server_address, storage_server_address;

int send_packet_to_naming_server(packet *packet_to_send, packet *packet_to_receive);
void interface_with_storage_server(packet *packet_to_send, int operation_to_execute, char *string_to_write);
void list_paths();
void access_file();
void create_delete_function();
void move_function();

#endif