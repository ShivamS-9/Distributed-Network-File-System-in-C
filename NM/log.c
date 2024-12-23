#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../packet.h"

#define log_file "log_file.txt"
#define MAX_LOG_LENGTH 256

void getIpAndPort(struct sockaddr *sa, char *ip, int *port)
{
    if (sa->sa_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)sa;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
        *port = ntohs(ipv4->sin_port);
    }
    else
    {
        strcpy(ip, "127.0.0.1");
        *port = 8080;
    }
}

void add_to_log_file(int client_or_ss, char *request, int ack, char *status, struct sockaddr *info, int i_port, char *i_ip)
{
    char ip[INET_ADDRSTRLEN]; // IPv4 only
    int port;

    if (i_port == -1 || strcmp(i_ip, "Struct") == 0)
    {
        getIpAndPort(info, ip, &port);
    }
    else
    {
        strcpy(ip, i_ip);
        port = i_port;
    }

    int fp = open(log_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fp == -1)
    {
        perror("Log\n");
        return;
    }

    char buf[MAX_LOG_LENGTH];
    for (int i = 0; i < MAX_LOG_LENGTH; i++)
    {
        buf[i] = '\0';
    }

    if (client_or_ss == CLIENT)
    {
        snprintf(buf, MAX_LOG_LENGTH, "[CLIENT] %s %d %s %s %d\n", request, ack, status, ip, port);
    }
    else if (client_or_ss == Storage_Server)
    {
        snprintf(buf, MAX_LOG_LENGTH, "[SS] %s %d %s %s %d\n", request, ack, status, ip, port);
    }

    printf("%s", buf);

    write(fp, buf, strlen(buf));

    close(fp);
}