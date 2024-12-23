#ifndef __HEADERS_H
#define __HEADERS_H

// Pre Defined Libraries

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>



// Pre Defined Macros

#define PATH_LEN 256


// Global Variables
extern int port_client;
extern int port_ns;
extern int port_univ;

// User implemented
#include "../packet.h"
#include "init.h"
#include "helper.h"
#include "client_handler.h"
#include "nm_handler.h"

// struct for multi-threading
typedef struct thread_cont
{
    packet* command;
    int * conn;
}cont_thread;

#endif