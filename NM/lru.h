#ifndef __LRU_H
#define __LRU_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CACHE 15

struct cache
{
    char path[MAX_PATH_LENGTH];
    int storage_server_index;
    time_t timestamp;
};

typedef struct cache cache;

void init_cache();
int least_recently_used();
void add_to_cache(char *path, int storage_server_index);
int get_storage_server_index(char *path);
void remove_from_cache(char *path);

// log
void add_to_log_file(int client_or_ss, char *request, int ack, char *status, struct sockaddr *info, int i_port, char *i_ip);
void getIpAndPort(struct sockaddr *sa, char *ip, int *port);

#endif