#ifndef __NM_HANDLER_H
#define __NM_HANDLER_H
#include "headers.h"

int copy_files(char* dir_target, int *con, packet* comm,char* path);
int create_directory(char* dir_target, int *con, packet* comm, char * dire_path);
void* process_req(void* arg);
void* ns_direct_con(void* arg);
void* ns_req(void* arg);

#endif