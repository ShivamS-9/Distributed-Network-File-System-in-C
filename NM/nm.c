#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Trie.h"
#include "../packet.h"
#include "lru.h"
#include <sys/time.h>
#include <time.h>

// Global Variables
#define NS_PORT_SS 8080 // Default Port Number for Naming Server
#define NS_PORT_C 8000
char server_ip_address[16]; // IP Address of Naming Server
// #define MAX_PATHS 100
#define MAX_SS 1000

// typedef struct ss
// {
//     int client_port;
//     int ns_port;
//     char ip_address[256];
//     char paths[MAX_PATHS][256];
//     int npaths;
//     int backup_index1;
//     int backup_index2;
//     int is_active;
//     int code;
// } ss_init;
typedef ns_init ss_init;

typedef struct arguments_client_thread
{
    int client_sock;
    struct sockaddr_in client_addr;
    socklen_t addr_size_c;
} arguments_client_thread;

typedef struct exec_args
{
    arguments_client_thread *args;
    packet Client_packet;
    int index;
    int backup_flag;
} exec_args;

typedef struct log
{
    char ip[256];
    int port;
    char msg[256];
} log;

ss_init SS[MAX_SS];
int SS_count = 0;
Trie *ALL_PATHS;
sem_t mutex;
sem_t backup_lock;

int acquire_readlock(char *path)
{
    Trie *temp = SearchTrie(path, ALL_PATHS);
    if (temp == NULL)
    {
        return -2;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 20;

    if (sem_timedwait(&(temp->read_write_mutex), &ts) == 0)
    {
        temp->read_count++;
        if (temp->read_count == 1)
        {
            sem_wait(&(temp->write_mutex));
        }
        sem_post(&(temp->read_write_mutex));
        return 0;
    }
    else
    {
        return -1;
    }
}

void release_readlock(char *path)
{
    Trie *temp = SearchTrie(path, ALL_PATHS);
    if (temp == NULL)
    {
        return;
    }
    sem_wait(&(temp->read_write_mutex));
    temp->read_count--;
    if (temp->read_count == 0)
    {
        sem_post(&(temp->write_mutex));
    }
    sem_post(&(temp->read_write_mutex));
}

int acquire_writelock(char *path)
{
    Trie *temp = SearchTrie(path, ALL_PATHS);
    if (temp == NULL)
    {
        return -2;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 20;

    if (sem_timedwait(&(temp->write_mutex), &ts) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void release_writelock(char *path)
{
    Trie *temp = SearchTrie(path, ALL_PATHS);
    if (temp == NULL)
    {
        return;
    }
    sem_post(&(temp->write_mutex));
}

int get_ss_index(char *path)
{
    // printf("Getting index\n");
    sem_wait(&mutex);
    int index = get_storage_server_index(path);
    printf("Index %d\n", index);
    if (index == -1)
    {
        // printf("Not found\n");
        Trie *temp = SearchTrie(path, ALL_PATHS);
        if (temp == NULL)
        {
            sem_post(&mutex);
            return -1;
        }
        index = temp->Exists;
        if (index == -1)
        {
            sem_post(&mutex);
            return -1;
        }
        add_to_cache(path, index);
    }
    sem_post(&mutex);
    // printf("Index hogaya\n");
    // If current SS not active
    return index;
    // sem_wait(&mutex);
    // if (SS[index].is_active == 0)
    // {
    //     if (SS[index].backup_index1 == -1)
    //     {
    //         ret_index = index;
    //     }
    //     else if (SS[SS[index].backup_index1].is_active == 0)
    //     {
    //         if (SS[index].backup_index2 == -1)
    //         {
    //             ret_index = index;
    //         }
    //         else if (SS[SS[index].backup_index2].is_active == 0)
    //         {
    //             printf("Server and backup server's crashed\n");
    //             sem_post(&mutex);
    //             // exit(1);
    //             return -1;
    //         }
    //         else
    //         {
    //             ret_index = SS[index].backup_index2;
    //         }
    //     }
    //     else
    //     {
    //         ret_index = SS[index].backup_index1;
    //     }
    // }
    // else
    // {
    //     ret_index = index;
    // }
    // sem_post(&mutex);
    // return ret_index;
}

struct indices
{
    int index;
    int index_dest;
};

void *backup_fxn(void *arg)
{
    struct indices INDEX = *((struct indices *)arg);
    int index = INDEX.index;
    int index_dest = INDEX.index_dest;
    int sock1;
    struct sockaddr_in addr1;
    socklen_t addr_size1;
    char buffer[1024];
    int n;

    sock1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock1 < 0)
    {
        perror("Socket error");
        // exit(1);
        // continue;
        return NULL;
    }
    sem_wait(&mutex);
    ss_init temp_ss = SS[index];
    sem_post(&mutex);

    memset(&addr1, '\0', sizeof(addr1));
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(temp_ss.ns_port);
    addr1.sin_addr.s_addr = inet_addr(temp_ss.ip_address);

    if (addr1.sin_addr.s_addr == -1)
    {
        perror("inet_addr error");
        // exit(1);
        // continue;
        return NULL;
    }

    if (connect(sock1, (struct sockaddr *)&addr1, sizeof(addr1)) == -1)
    {
        // printf("Bhajiya");
        perror("Connect Error");
        // exit(1);
        // continue;
        return NULL;
    }
    // Recv message and send msg kya hoga ss se, abhi string rakha hai baadmai will add jo bhi struct chaiye rahega
    // bzero(buffer, 1024);

    // connect
    packet Return_packet;
    // Return_packet.ns_port = temp_ss.ns_port;
    // strcpy(Return_packet.ns_ip_address, "127.0.0.1");
    // Return_packet.ack = 1
    Return_packet.operation_code = BACKUP;
    Return_packet.file_or_folder_code = FOLDER_TYPE;
    strcpy(Return_packet.contents, "");
    for (int i = 0; i < 4096; i++)
    {
        Return_packet.contents[i] = '\0';
    }
    strcpy(Return_packet.destination_path_name, ".");
    strcpy(Return_packet.source_path_name, ".");
    if (send(sock1, (void *)&Return_packet, sizeof(packet), 0) == -1)
    {
        perror("Send error");
        // exit(1);
        // continue;
        return NULL;
    }
    // int index_dest = get_ss_index(Client_packet.destination_path_name);
    sem_wait(&mutex);
    temp_ss = SS[index_dest];
    // printf("L%dL", index_dest);
    if (temp_ss.is_active == 1)
    {
        sem_post(&mutex);
    }
    else
    {
        sem_post(&mutex);
        printf("Backup Server down\n");
        return NULL;
        // return NULL;
        // continue;
    }
    // char *ip = "127.0.0.1";

    int sock2;
    struct sockaddr_in addr2;
    socklen_t addr_size2;
    // char buffer[1024];
    // int n;

    sock2 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock2 < 0)
    {
        perror("Socket error");
        // exit(1);
        // continue;
        return NULL;
    }

    memset(&addr2, '\0', sizeof(addr2));
    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(temp_ss.ns_port);
    // printf("I %d", temp_ss.ns_port);
    addr2.sin_addr.s_addr = inet_addr(temp_ss.ip_address);

    if (addr2.sin_addr.s_addr == -1)
    {
        perror("inet_addr error");
        // exit(1);
        // continue;
        return NULL;
    }

    if (connect(sock2, (struct sockaddr *)&addr2, sizeof(addr2)) == -1)
    {
        perror("Connect Error");
        // exit(1);
        // continue;
        return NULL;
    }
    // int flag = 1;
    // if (Client_packet.file_or_folder_code == FOLDER_TYPE)
    // {
    while (1)
    {
        // printf("Lesgoo\n");
        // fflush(stdout);
        int ret = recv(sock1, (void *)&Return_packet, sizeof(packet), 0);
        char buff_temp[256];
        if (strncmp(Return_packet.destination_path_name, ".//", 3) == 0)
        {
            strcpy(buff_temp, "./");
            strcat(buff_temp, &(Return_packet.destination_path_name[3]));
            strcpy(Return_packet.destination_path_name, buff_temp);
        }
        if (strncmp(Return_packet.source_path_name, ".//", 3) == 0)
        {
            strcpy(buff_temp, "./");
            strcat(buff_temp, &(Return_packet.source_path_name[3]));
            strcpy(Return_packet.source_path_name, buff_temp);
        }
        if (strncmp(Return_packet.contents, ".//", 3) == 0)
        {
            strcpy(buff_temp, "./");
            strcat(buff_temp, &(Return_packet.contents[3]));
            strcpy(Return_packet.contents, buff_temp);
        }
        // printf("a%sa\n", Return_packet.destination_path_name);
        // printf("c%sc\n", Return_packet.source_path_name);
        // printf("d%sd\n", Return_packet.contents);
        // printf("b%db\n", Return_packet.operation_code);
        if (ret == -1)
        {
            perror("Recv Error");
            exit(1);
            // continue;
        }
        if (ret == 0)
        {
            printf("SS disconnected\n");
            // continue;
            break;
        }
        if (Return_packet.operation_code == STOP)
        {
            // close(sock1);
            // close(sock2);
            // Return_packet.ack = 1;
            // char buf[256];
            // Insert in trie
            // strcpy(buf, Return_packet.destination_path_name);
            // sem_wait(&mutex);
            // InsertTrie(buf, ALL_PATHS, index_dest);
            // sem_post(&mutex);
            // if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
            // {
            //     perror("Send error");
            //     // exit(1);
            //     continue;
            // }
            printf("Dir's done\n");
            fflush(stdout);
            break;
        }
        // char temp[256];
        // strcpy(temp, Return_packet.source_path_name);
        // strcat(temp, "/");
        // strcat(temp, Return_packet.contents);
        // temp[strlen(temp) - 4] = '\0';
        // strcat(temp, "_copy.txt");
        // sem_wait(&mutex);
        // printf("V%sV",temp);
        // InsertTrie(temp, ALL_PATHS, index_dest);
        // sem_post(&mutex);
        if (send(sock2, (void *)&Return_packet, sizeof(packet), 0) == -1)
        {
            perror("Send Error");
            continue;
        }
        ret = recv(sock2, (void *)&Return_packet, sizeof(packet), 0);
        if (ret == 0)
        {
            printf("SS closed\n");
        }
        if (ret == -1)
        {
            perror("Recv Error");
            exit(1);
        }
        if (Return_packet.operation_code == STOP)
        {
            // sem_wait(&mutex);
            // char temp[256];
            // strcpy(temp, Return_packet.source_path_name);
            // strcat(temp, "/");
            // strcat(temp, Return_packet.contents);
            // printf("V%sV", temp);
            // InsertTrie(temp, ALL_PATHS, index);
            // sem_post(&mutex);
            Return_packet.ack == 1;
            printf("CREATE done\n");
        }
        else
        {
            close(sock1);
            close(sock2);
            // break;
            perror("Error\n");
            exit(1);
            // close(sock1);
            // close(sock2);
            // Return_packet.ack = 0;
            // Return_packet.operation_code = -2;
            // if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
            // {
            //     perror("Send error");
            //     // exit(1);
            //     continue;
            // }
            // printf("Error hai\n");
        }
    }
    int flag = 1;
    int flagging = 0;
    int MOV_COUNT = 0;
    int MOV_INDEX = 0;
    int STOP_COUNT = 0;
    while (1)
    {
        // printf("File\n");
        packet FILESYAY[30];
        // int STOP_COUNT = 0;
        int ret = recv(sock1, (void *)&Return_packet, sizeof(packet), 0);
        char buff_temp[256];
        if (strncmp(Return_packet.destination_path_name, ".//", 3) == 0)
        {
            strcpy(buff_temp, "./");
            strcat(buff_temp, &(Return_packet.destination_path_name[3]));
            strcpy(Return_packet.destination_path_name, buff_temp);
        }
        if (strncmp(Return_packet.source_path_name, ".//", 3) == 0)
        {
            strcpy(buff_temp, "./");
            strcat(buff_temp, &(Return_packet.source_path_name[3]));
            strcpy(Return_packet.source_path_name, buff_temp);
        }
        if (strncmp(Return_packet.contents, ".//", 3) == 0)
        {
            strcpy(buff_temp, "./");
            strcat(buff_temp, &(Return_packet.contents[3]));
            strcpy(Return_packet.contents, buff_temp);
        }
        // printf("a%sa\n", Return_packet.destination_path_name);
        // printf("c%sc\n", Return_packet.source_path_name);
        // printf("d%sd\n", Return_packet.contents);
        // printf("b%db\n", Return_packet.operation_code);
        if (flag == 1)
        {
            if ((Return_packet.operation_code==STOP_CPY)&&(MOV_COUNT==0)&&(STOP_COUNT==0))
            {
                return NULL;
            }
            FILESYAY[MOV_INDEX] = Return_packet;
            MOV_COUNT++;
            flag = 0;
            Return_packet.operation_code = STOP;
            STOP_COUNT++;
        }
        if (ret == -1)
        {
            perror("Recv Error");
            exit(1);
            // continue;
        }
        if (ret == 0)
        {
            printf("SS disconnected\n");
            // continue;
            break;
        }
        if (Return_packet.operation_code == STOP_CPY)
        {
            flagging = 1;
            // kuch karo
        }
        else if (Return_packet.operation_code == STOP)
        {
            if (STOP_COUNT == MOV_COUNT + 1 && flagging == 1)
            {
                Return_packet.ack = 1;
                Return_packet.operation_code = SUCCESS;
                // if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                // {
                //     perror("Send error");
                //     // exit(1);
                //     continue;
                // }
                break;
                // exit now ig
            }
            if (send(sock1, (void *)&FILESYAY[MOV_INDEX], sizeof(packet), 0) == -1)
            {
                perror("Send Error");
                continue;
            }
            STOP_COUNT++;
            MOV_INDEX++;
        }
        else if (Return_packet.operation_code == BACKUP)
        {
            // char temp[256];
            // strcpy(temp, Return_packet.destination_path_name);
            // strcat(temp, "/");
            // strcat(temp, Return_packet.contents);
            // temp[strlen(temp) - 4] = '\0';
            // strcat(temp, "_copy.txt");
            // printf("V%sV", temp);
            // sem_wait(&mutex);
            // InsertTrie(temp, ALL_PATHS, index_dest);
            // sem_post(&mutex);
            FILESYAY[MOV_COUNT] = Return_packet;
            MOV_COUNT++;
        }
        else if ((Return_packet.operation_code == APPEND) || (Return_packet.operation_code == WRITE))
        {
            if (send(sock2, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send Error");
                continue;
            }
            int ret1 = recv(sock2, (void *)&Return_packet, sizeof(packet), 0);
            if (ret1 == 0)
            {
                printf("Abruptly closes\n");
                exit(1);
            }
            if (ret1 == -1)
            {
                perror("Recv");
                exit(1);
            }
            if (Return_packet.operation_code == -2)
            {
            }
            else
            {
                close(sock1);
                close(sock2);
                Return_packet.ack = 0;
                Return_packet.operation_code = -2;
                // if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                // {
                //     perror("Send error");
                //     // exit(1);
                //     continue;
                // }
                printf("Error hai\n");
            }
        }
        else
        {
            // printf("U%dU", Return_packet.operation_code);
            // printf("Bhajiya\n");
            // exit(1);
            return NULL;
            if (send(sock1, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send Error");
                continue;
            }
        }
    }
    // }
    // if (Client_packet.file_or_folder_code == FILE_TYPE)
    // {
    //     while (1)
    //     {
    //         // printf("Kya\n");
    //         int ret = recv(sock1, (void *)&Return_packet, sizeof(packet), 0);
    //         if (ret == -1)
    //         {
    //             perror("Recv Error");
    //             exit(1);
    //             // continue;
    //         }
    //         if (ret == 0)
    //         {
    //             printf("SS disconnected\n");
    //             // continue;
    //             break;
    //         }
    //         if (Return_packet.operation_code == STOP)
    //         {
    //             close(sock1);
    //             close(sock2);
    //             Return_packet.ack = 1;
    //             char buf[256];
    //             // Insert in trie
    //             printf("aa%saa\n", Return_packet.destination_path_name);
    //             strcpy(buf, Return_packet.destination_path_name);
    //             sem_wait(&mutex);
    //             InsertTrie(buf, ALL_PATHS, index_dest);
    //             sem_post(&mutex);
    //             if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
    //             {
    //                 perror("Send error");
    //                 // exit(1);
    //                 continue;
    //             }
    //             break;
    //         }
    //         if (send(sock2, (void *)&Return_packet, sizeof(packet), 0) == -1)
    //         {
    //             perror("Send Error");
    //             continue;
    //         }
    //         ret = recv(sock2, (void *)&Return_packet, sizeof(packet), 0);
    //         if (ret == 0)
    //         {
    //             printf("SS closed\n");
    //         }
    //         if (ret == -1)
    //         {
    //             perror("Recv Error");
    //             exit(1);
    //         }
    //         if (Return_packet.operation_code == -2)
    //         {
    //         }
    //         else
    //         {
    //             close(sock1);
    //             close(sock2);
    //             Return_packet.ack = 0;
    //             Return_packet.operation_code = -2;
    //             if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
    //             {
    //                 perror("Send error");
    //                 // exit(1);
    //                 continue;
    //             }
    //             printf("Error hai\n");
    //         }
    //     }
    //     printf("Moved\n");
    // }
}

void *executing(void *arg)
{
    exec_args *argumentings = arg;
    arguments_client_thread *args = argumentings->args;
    packet Client_packet = argumentings->Client_packet;
    int index = argumentings->index;
    int backup_flag = argumentings->backup_flag;
    // write scam hai yaar
    if (Client_packet.operation_code == WRITE)
    {
        if (backup_flag == 0)
        {
            sem_wait(&mutex);
            ss_init temp_ss = SS[index];
            sem_post(&mutex);
            packet Return_packet;
            Return_packet.ns_port = temp_ss.client_port;
            strcpy(Return_packet.ns_ip_address, temp_ss.ip_address);
            Return_packet.ack = 1;
            Return_packet.operation_code = WRITE;
            // printf("Hello\n");
            // printf("b%db", args->client_sock);
            int write_lock_ret = acquire_writelock(Client_packet.source_path_name);
            if (write_lock_ret == -1)
            {
                packet SS_down;
                SS_down.ack = 0;
                SS_down.operation_code = TIME_OUT;
                send(args->client_sock, (void *)&SS_down, sizeof(packet), 0);
                add_to_log_file(CLIENT, "WRITE", 0, "Time Out", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                return NULL;
            }
            else if (write_lock_ret == -2)
            {
                packet SS_down;
                SS_down.ack = 0;
                SS_down.operation_code = NOT_FOUND;
                send(args->client_sock, (void *)&SS_down, sizeof(packet), 0);
                add_to_log_file(CLIENT, "WRITE", 0, "Not Found", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                return NULL;
            }
            if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send error");
                release_writelock(Client_packet.source_path_name);
                exit(1);
            }
            printf("sent port\n");
            // printf("b%db", args->client_sock);
            int ret = recv(args->client_sock, (void *)&Return_packet, sizeof(packet), 0);
            if (ret == -1)
            {
                perror("Recv Error");
                add_to_log_file(CLIENT, "WRITE", 0, "Recv Error", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                release_writelock(Client_packet.source_path_name);
                // exit(1);
                return NULL;
            }
            else if (ret == 0)
            {
                printf("Client Disconnected\n");
                add_to_log_file(CLIENT, "WRITE", 0, "Client Disconnected", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                release_writelock(Client_packet.source_path_name);
                return NULL;
            }
            if (Return_packet.operation_code == STOP)
            {
                add_to_log_file(CLIENT, "WRITE", 0, "Success", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                add_to_log_file(Storage_Server, "WRITE", 0, "Success", (struct sockaddr *)&(args->client_addr), index, "Struct");
                printf("Read oper done");
            }
            release_writelock(Client_packet.source_path_name);
        }
        if (backup_flag == 1)
        {
            printf("Sahi Jagah pe aya\n");
            sem_wait(&mutex);
            ss_init temp_ss = SS[index];
            sem_post(&mutex);
            packet Return_packet;
            Return_packet.ns_port = temp_ss.ns_port;
            strcpy(Return_packet.ns_ip_address, temp_ss.ip_address);
            Return_packet.ack = 1;
            Return_packet.operation_code = WRITE;
            char temp[256];
            strcpy(Return_packet.destination_path_name, Client_packet.source_path_name);
            strcpy(Return_packet.contents, Client_packet.contents);

            int sock1;
            struct sockaddr_in addr1;
            socklen_t addr_size1;
            char buffer[1024];
            int n;

            sock1 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock1 < 0)
            {
                perror("Socket error");
                // exit(1);
                // continue;
                return NULL;
            }

            memset(&addr1, '\0', sizeof(addr1));
            addr1.sin_family = AF_INET;
            addr1.sin_port = htons(temp_ss.ns_port);
            // printf("K%dK", temp_ss.ns_port);
            addr1.sin_addr.s_addr = inet_addr(temp_ss.ip_address);

            if (addr1.sin_addr.s_addr == -1)
            {
                perror("inet_addr error");
                // exit(1);
                // continue;
                return NULL;
            }

            if (connect(sock1, (struct sockaddr *)&addr1, sizeof(addr1)) == -1)
            {
                perror("Connect Error");
                // exit(1);
                // continue;
                return NULL;
            }
            // Recv message and send msg kya hoga ss se, abhi string rakha hai baadmai will add jo bhi struct chaiye rahega
            // bzero(buffer, 1024);

            // connect
            // packet Return_packet;
            // Return_packet.ns_port = temp_ss.ns_port;
            // strcpy(Return_packet.ns_ip_address, "127.0.0.1");
            // Return_packet.ack = 1
            // Return_packet = Client_packet;
            Return_packet.operation_code = WRITE;
            sem_wait(&backup_lock);
            if (send(sock1, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send error");
                // exit(1);
                return NULL;
            }
            int ret_value = recv(sock1,(void*)&Return_packet,sizeof(packet),0);
            sem_post(&backup_lock);
            if (ret_value<0)
            {
                perror("Recv error");
                // exit(1);
                return NULL;
            }
            if (ret_value==0)
            {
                printf("SS closed\n");
                return NULL;
            }
            if (Return_packet.operation_code==STOP)
            {
                
            }
        }
    }
    else if (Client_packet.operation_code == MOVE)
    {
        sem_wait(&mutex);
        ss_init temp_ss = SS[index];
        // printf("M%dM", index);
        if (temp_ss.is_active == 1)
        {
            sem_post(&mutex);
        }
        else
        {
            sem_post(&mutex);
            printf("Scam\n");
            return NULL;
            // continue;
        }
        // char *ip = "127.0.0.1";

        int sock1;
        struct sockaddr_in addr1;
        socklen_t addr_size1;
        char buffer[1024];
        int n;

        sock1 = socket(AF_INET, SOCK_STREAM, 0);
        if (sock1 < 0)
        {
            perror("Socket error");
            // exit(1);
            // continue;
            return NULL;
        }

        memset(&addr1, '\0', sizeof(addr1));
        addr1.sin_family = AF_INET;
        addr1.sin_port = htons(temp_ss.ns_port);
        // printf("K%dK", temp_ss.ns_port);
        addr1.sin_addr.s_addr = inet_addr(temp_ss.ip_address);

        if (addr1.sin_addr.s_addr == -1)
        {
            perror("inet_addr error");
            // exit(1);
            // continue;
            return NULL;
        }

        if (connect(sock1, (struct sockaddr *)&addr1, sizeof(addr1)) == -1)
        {
            perror("Connect Error");
            // exit(1);
            // continue;
            return NULL;
        }
        // Recv message and send msg kya hoga ss se, abhi string rakha hai baadmai will add jo bhi struct chaiye rahega
        // bzero(buffer, 1024);

        // connect
        packet Return_packet;
        // Return_packet.ns_port = temp_ss.ns_port;
        // strcpy(Return_packet.ns_ip_address, "127.0.0.1");
        // Return_packet.ack = 1
        Return_packet = Client_packet;
        Return_packet.operation_code = MOVE;
        if (send(sock1, (void *)&Return_packet, sizeof(packet), 0) == -1)
        {
            perror("Send error");
            // exit(1);
            return NULL;
        }
        int index_dest = get_ss_index(Client_packet.destination_path_name);
        if (index_dest == -1)
        {
            printf("Invalid Path\n");
            // shayad send karna pade packet
            return NULL;
        }
        sem_wait(&mutex);
        temp_ss = SS[index_dest];
        // printf("Y%dY", index_dest);
        if (temp_ss.is_active == 1)
        {
            sem_post(&mutex);
        }
        else
        {
            sem_post(&mutex);
            printf("Scam\n");
            // return NULL;
            return NULL;
        }
        // char *ip = "127.0.0.1";

        int sock2;
        struct sockaddr_in addr2;
        socklen_t addr_size2;
        // char buffer[1024];
        // int n;

        sock2 = socket(AF_INET, SOCK_STREAM, 0);
        if (sock2 < 0)
        {
            perror("Socket error");
            // exit(1);
            return NULL;
        }

        memset(&addr2, '\0', sizeof(addr2));
        addr2.sin_family = AF_INET;
        addr2.sin_port = htons(temp_ss.ns_port);
        // printf("pls %d\n", temp_ss.ns_port);
        fflush(stdout);
        addr2.sin_addr.s_addr = inet_addr(temp_ss.ip_address);

        if (addr2.sin_addr.s_addr == -1)
        {
            perror("inet_addr error");
            // exit(1);
            return NULL;
        }

        if (connect(sock2, (struct sockaddr *)&addr2, sizeof(addr2)) == -1)
        {
            perror("Connect Error");
            // exit(1);
            return NULL;
        }
        // int flag = 1;
        if (Client_packet.file_or_folder_code == FOLDER_TYPE)
        {
            while (1)
            {
                int ret = recv(sock1, (void *)&Return_packet, sizeof(packet), 0);
                if (ret == -1)
                {
                    perror("Recv Error");
                    exit(1);
                    //  return NULL;
                }
                if (ret == 0)
                {
                    printf("SS disconnected\n");
                    //  return NULL;
                    break;
                }
                if (Return_packet.operation_code == STOP)
                {
                    // close(sock1);
                    // close(sock2);
                    // Return_packet.ack = 1;
                    // char buf[256];
                    // Insert in trie
                    // strcpy(buf, Return_packet.destination_path_name);
                    // sem_wait(&mutex);
                    // InsertTrie(buf, ALL_PATHS, index_dest);
                    // sem_post(&mutex);
                    // if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                    // {
                    //     perror("Send error");
                    //     // exit(1);
                    //      return NULL;
                    // }
                    printf("Dir's done\n");
                    fflush(stdout);
                    break;
                }
                // char temp[256];
                // strcpy(temp, Return_packet.source_path_name);
                // strcat(temp, "/");
                // strcat(temp, Return_packet.contents);
                // temp[strlen(temp) - 4] = '\0';
                // strcat(temp, "_copy.txt");
                // sem_wait(&mutex);
                // printf("V%sV",temp);
                // InsertTrie(temp, ALL_PATHS, index_dest);
                // sem_post(&mutex);
                if (send(sock2, (void *)&Return_packet, sizeof(packet), 0) == -1)
                {
                    perror("Send Error");
                    return NULL;
                }
                ret = recv(sock2, (void *)&Return_packet, sizeof(packet), 0);
                if (ret == 0)
                {
                    printf("SS closed\n");
                }
                if (ret == -1)
                {
                    perror("Recv Error");
                    exit(1);
                }
                if (Return_packet.operation_code == STOP)
                {
                    sem_wait(&mutex);
                    char temp[256];
                    strcpy(temp, Return_packet.source_path_name);
                    strcat(temp, "/");
                    strcat(temp, Return_packet.contents);
                    // printf("V%sV", temp);
                    printf("str : %s added\n",temp);
                    InsertTrie(temp, ALL_PATHS, index);
                    sem_post(&mutex);
                    Return_packet.ack == 1;
                    printf("CREATE done\n");
                }
                else
                {
                    close(sock1);
                    close(sock2);
                    // break;
                    perror("Error\n");
                    exit(1);
                    // close(sock1);
                    // close(sock2);
                    // Return_packet.ack = 0;
                    // Return_packet.operation_code = -2;
                    // if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                    // {
                    //     perror("Send error");
                    //     // exit(1);
                    //      return NULL;
                    // }
                    // printf("Error hai\n");
                }
            }
            int flag = 1;
            int flagging = 0;
            int MOV_COUNT = 0;
            int MOV_INDEX = 0;
            int STOP_COUNT = 0;
            while (1)
            {
                // printf("File\n");
                packet FILESYAY[30];
                // int STOP_COUNT = 0;
                int ret = recv(sock1, (void *)&Return_packet, sizeof(packet), 0);
                // printf("a%sa\n",Return_packet.destination_path_name);
                // printf("c%sc\n",Return_packet.source_path_name);
                // printf("d%sd\n",Return_packet.contents);
                // printf("b%db\n",Return_packet.operation_code);
                if (flag == 1)
                {
                    FILESYAY[MOV_INDEX] = Return_packet;
                    MOV_COUNT++;
                    flag = 0;
                    Return_packet.operation_code = STOP;
                    STOP_COUNT++;
                }
                if (ret == -1)
                {
                    perror("Recv Error");
                    exit(1);
                    //  return NULL;
                }
                if (ret == 0)
                {
                    printf("SS disconnected\n");
                    //  return NULL;
                    break;
                }
                if (Return_packet.operation_code == STOP_CPY)
                {
                    flagging = 1;
                    // kuch karo
                }
                else if (Return_packet.operation_code == STOP)
                {
                    if (STOP_COUNT == MOV_COUNT + 1 && flagging == 1)
                    {
                        Return_packet.ack = 1;
                        Return_packet.operation_code = SUCCESS;
                        if (backup_flag == 0)
                        {
                            if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                            {
                                perror("Send error");
                                // exit(1);
                                return NULL;
                            }
                        }
                        break;
                        // exit now ig
                    }
                    if (send(sock1, (void *)&FILESYAY[MOV_INDEX], sizeof(packet), 0) == -1)
                    {
                        perror("Send Error");
                        return NULL;
                    }
                    STOP_COUNT++;
                    MOV_INDEX++;
                }
                else if (Return_packet.operation_code == MOVE)
                {
                    char temp[256];
                    strcpy(temp, Return_packet.destination_path_name);
                    strcat(temp, "/");
                    strcat(temp, Return_packet.contents);
                    temp[strlen(temp) - 4] = '\0';
                    strcat(temp, "_copy.txt");
                    // printf("V%sV", temp);
                    sem_wait(&mutex);
                    printf("str : %s added\n",temp);
                    InsertTrie(temp, ALL_PATHS, index_dest);
                    sem_post(&mutex);
                    FILESYAY[MOV_COUNT] = Return_packet;
                    MOV_COUNT++;
                }
                else if ((Return_packet.operation_code == APPEND) || (Return_packet.operation_code == WRITE))
                {
                    if (send(sock2, (void *)&Return_packet, sizeof(packet), 0) == -1)
                    {
                        perror("Send Error");
                        return NULL;
                    }
                    int ret1 = recv(sock2, (void *)&Return_packet, sizeof(packet), 0);
                    if (ret1 == 0)
                    {
                        printf("Abruptly closes\n");
                        exit(1);
                    }
                    if (ret1 == -1)
                    {
                        perror("Recv");
                        exit(1);
                    }
                    if (Return_packet.operation_code == -2)
                    {
                    }
                    else
                    {
                        close(sock1);
                        close(sock2);
                        Return_packet.ack = 0;
                        Return_packet.operation_code = -2;
                        if (backup_flag == 0)
                        {
                            if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                            {
                                perror("Send error");
                                // exit(1);
                                return NULL;
                            }
                        }
                        printf("Error hai\n");
                    }
                }
                else
                {
                    printf("Y%dY", Return_packet.operation_code);
                    printf("Bhajiya\n");
                    exit(1);
                    if (send(sock1, (void *)&Return_packet, sizeof(packet), 0) == -1)
                    {
                        perror("Send Error");
                        return NULL;
                    }
                }
            }
        }
        if (Client_packet.file_or_folder_code == FILE_TYPE)
        {
            while (1)
            {
                int ret = recv(sock1, (void *)&Return_packet, sizeof(packet), 0);
                if (ret == -1)
                {
                    perror("Recv Error");
                    exit(1);
                    //  return NULL;
                }
                if (ret == 0)
                {
                    printf("SS disconnected\n");
                    //  return NULL;
                    break;
                }
                printf("Kya\n");
                fflush(stdout);
                if (Return_packet.operation_code == STOP)
                {
                    close(sock1);
                    close(sock2);
                    Return_packet.ack = 1;
                    char buf[256];
                    // Insert in trie
                    printf("aa%saa\n", Return_packet.destination_path_name);
                    strcpy(buf, Return_packet.destination_path_name);
                    sem_wait(&mutex);
                    printf("str : %s added\n",buf);
                    InsertTrie(buf, ALL_PATHS, index_dest);
                    sem_post(&mutex);
                    if (backup_flag == 0)
                    {
                        if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                        {
                            perror("Send error");
                            // exit(1);
                            return NULL;
                        }
                    }
                    break;
                }
                if (send(sock2, (void *)&Return_packet, sizeof(packet), 0) == -1)
                {
                    perror("Send Error");
                    return NULL;
                }
                ret = recv(sock2, (void *)&Return_packet, sizeof(packet), 0);
                if (ret == 0)
                {
                    printf("SS closed\n");
                }
                if (ret == -1)
                {
                    perror("Recv Error");
                    exit(1);
                }
                printf("COWTEST");
                fflush(stdout);
                if (Return_packet.operation_code == -2)
                {
                }
                else
                {
                    close(sock1);
                    close(sock2);
                    Return_packet.ack = 0;
                    Return_packet.operation_code = -2;
                    if (backup_flag == 0)
                    {
                        if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                        {
                            perror("Send error");
                            // exit(1);
                            return NULL;
                        }
                    }
                    printf("Error hai\n");
                }
            }
            printf("Moved\n");
        }
    }
    else if (Client_packet.operation_code == DELETE)
    {
        sem_wait(&mutex);
        ss_init temp_ss = SS[index];
        if (temp_ss.is_active == 1)
        {
            sem_post(&mutex);
        }
        else
        {
            sem_post(&mutex);
            printf("Scam\n");
            // return NULL;
            return NULL;
        }
        // char *ip = "127.0.0.1";

        int sock;
        struct sockaddr_in addr;
        socklen_t addr_size;
        char buffer[1024];
        int n;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("Socket error");
            // exit(1);
            return NULL;
        }

        memset(&addr, '\0', sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(temp_ss.ns_port);
        addr.sin_addr.s_addr = inet_addr(temp_ss.ip_address);

        if (addr.sin_addr.s_addr == -1)
        {
            perror("inet_addr error");
            // exit(1);
            return NULL;
        }

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("Connect Error");
            // exit(1);
            return NULL;
        }
        // Recv message and send msg kya hoga ss se, abhi string rakha hai baadmai will add jo bhi struct chaiye rahega
        // bzero(buffer, 1024);

        // connect
        packet Return_packet;
        // Return_packet.ns_port = temp_ss.ns_port;
        // strcpy(Return_packet.ns_ip_address, "127.0.0.1");
        // Return_packet.ack = 1
        Return_packet = Client_packet;
        Return_packet.operation_code = DELETE;
        if (send(sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
        {
            perror("Send error");
            // exit(1);
            return NULL;
        }
        int ret = recv(sock, (void *)&Return_packet, sizeof(packet), 0);
        if (ret == -1)
        {
            add_to_log_file(CLIENT, "DELETE", 0, "Recv Error", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "DELETE", 0, "Recv Error", NULL, temp_ss.ns_port, temp_ss.ip_address);
            perror("Recv Error");
            // exit(1)
            return NULL;
        }
        if (ret == 0)
        {
            add_to_log_file(CLIENT, "DELETE", 0, "SS Disconnected", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "DELETE", 0, "SS Disconnected", NULL, temp_ss.ns_port, temp_ss.ip_address);
            printf("SS disconnected\n");
            return NULL;
        }
        // printf("l%dl", Return_packet.operation_code);
        fflush(stdout);
        if (Return_packet.operation_code == STOP)
        {
            Return_packet.ack == 1;
            sem_wait(&mutex);
            if (Return_packet.file_or_folder_code == FILE_TYPE)
            {
                char temp[256];
                strcpy(temp, Return_packet.source_path_name);
                strcat(temp, "/");
                strcat(temp, Return_packet.contents);
                Delete(temp, ALL_PATHS);
                char buff[256];
                // PrintTrie(ALL_PATHS, 0, buff);
            }
            else
            {
                // Delete(Return_packet.source_path_name, ALL_PATHS);
                char temp[256];
                strcpy(temp, Return_packet.source_path_name);
                strcat(temp, "/");
                strcat(temp, Return_packet.contents);
                Delete(temp, ALL_PATHS);
                char buff[256];
                strcat(temp, "/");
                Delete_all_with_prefix(Return_packet.source_path_name, ALL_PATHS);
                // PrintTrie(ALL_PATHS, 0, buff);
            }
            sem_post(&mutex);
            if (backup_flag == 0)
            {
                if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                {
                    perror("Send error");
                    // exit(1);
                    return NULL;
                }
            }
            add_to_log_file(CLIENT, "DELETE", 1, "Success", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "DELETE", 1, "Success", NULL, temp_ss.ns_port, temp_ss.ip_address);
            printf("DELETE done\n");
        }
        else
        {
            add_to_log_file(CLIENT, "DELETE", 0, "Error", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "DELETE", 0, "Error", NULL, temp_ss.ns_port, temp_ss.ip_address);
            Return_packet.ack = 0;
            printf("Error\n");
            if (backup_flag == 0)
            {
                if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                {
                    perror("Send error");
                    // exit(1);
                    return NULL;
                }
            }
        }
    }
    else if (Client_packet.operation_code == CREATE)
    {
        sem_wait(&mutex);
        ss_init temp_ss = SS[index];
        if (temp_ss.is_active == 1)
        {
            sem_post(&mutex);
        }
        else
        {
            sem_post(&mutex);
            printf("Scam\n");
            return NULL;
        }
        char *ip = "127.0.0.1";

        int sock;
        struct sockaddr_in addr;
        socklen_t addr_size;
        char buffer[1024];
        int n;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("Socket error");
            // exit(1);
            return NULL;
        }

        memset(&addr, '\0', sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(temp_ss.ns_port);
        addr.sin_addr.s_addr = inet_addr(temp_ss.ip_address);
        if (addr.sin_addr.s_addr == -1)
        {
            perror("inet_addr error");
            // exit(1);
            return NULL;
        }

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("Connect Error");
            // exit(1);
            return NULL;
        }
        // Recv message and send msg kya hoga ss se, abhi string rakha hai baadmai will add jo bhi struct chaiye rahega
        // bzero(buffer, 1024);

        // connect
        packet Return_packet;
        // Return_packet.ns_port = temp_ss.ns_port;
        // strcpy(Return_packet.ns_ip_address, "127.0.0.1");
        // Return_packet.ack = 1
        Return_packet = Client_packet;
        // printf("l%dl",Return_packet.file_or_folder_code);
        // printf("Bhai sahab");
        if (send(sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
        {
            perror("Send error");
            // exit(1);
            return NULL;
        }
        int ret = recv(sock, (void *)&Return_packet, sizeof(packet), 0);
        if (ret == -1)
        {
            add_to_log_file(CLIENT, "CREATE", 0, "Recv Error", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "CREATE", 0, "Recv Error", NULL, temp_ss.ns_port, temp_ss.ip_address);
            perror("Recv Error");
            // exit(1)
            return NULL;
        }
        if (ret == 0)
        {
            add_to_log_file(CLIENT, "CREATE", 0, "SS Disconnected", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "CREATE", 0, "SS Disconnected", NULL, temp_ss.ns_port, temp_ss.ip_address);
            printf("SS disconnected\n");
            return NULL;
        }
        if (Return_packet.operation_code == STOP)
        {
            sem_wait(&mutex);
            char temp[256];
            strcpy(temp, Return_packet.source_path_name);
            strcat(temp, "/");
            strcat(temp, Return_packet.contents);
            printf("str : %s added\n",temp);
            InsertTrie(temp, ALL_PATHS, index);
            sem_post(&mutex);
            Return_packet.ack == 1;
            add_to_log_file(CLIENT, "CREATE", 1, "Success", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "CREATE", 1, "Success", NULL, temp_ss.ns_port, temp_ss.ip_address);
            printf("CREATE done\n");
        }
        if (Return_packet.operation_code == UNAUTHORIZED)
        {
            add_to_log_file(CLIENT, "CREATE", 0, "Unauthorized Access", (struct sockaddr *)&(args->client_addr), -1, "Struct");
            add_to_log_file(Storage_Server, "CREATE", 0, "Unauthorized Access", NULL, temp_ss.ns_port, temp_ss.ip_address);
            Return_packet.ack = 0;
            Return_packet.operation_code = UNAUTHORIZED;
        }
        if (backup_flag == 0)
        {
            if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send error");
                // exit(1);
                return NULL;
            }
        }
    }
}

// Established ns connection with ss on ss port, from cmd given by client
void *ns_ss_called_by_client(void *arg)
{
    arguments_client_thread *args = arg;
    packet Client_packet;
    // Since haven't written code for establishing connection, with client, assuming command is recieved and reaches here
    // Once there's a clear structure for how the client command will come i'll add this function in the client thread only

    // if this gives error blame PK
    while (1)
    {
        // printf("WOW");
        fflush(stdout);
        int val = recv(args->client_sock, (void *)&Client_packet, sizeof(packet), 0);
        if (val == -1)
        {
            perror("Recv Error");
            // exit(1);
            return NULL;
        }
        else if (val == 0)
        {
            printf("Client Discsonnected\n");
            return NULL;
        }
        // printf("Hello %d\n", Client_packet.operation_code);
        int index;
        char temp[256];
        if (Client_packet.operation_code == LIST)
        {
            strcpy(temp, Client_packet.source_path_name);
            temp[strlen(temp) - 1] = '\0';
            index = get_ss_index(temp);
        }
        else
        {
            index = get_ss_index(Client_packet.source_path_name);
        }
        if (index == -1)
        {
            printf("Invalid path\n");
            packet Return_packet;
            Return_packet.operation_code = NOT_FOUND;
            Return_packet.ack = 0;
            if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send error");
                // exit(1);
                continue;
            }
            // int ret=recv(args->client_sock, (void *)&Return_packet, sizeof(packet), 0);
            // if (ret==-1)
            // {
            //     perror("Recv error");
            //     return NULL;
            // }
            // if (ret==0)
            // {
            //     printf("Client Closed\n");
            //     return NULL;
            // }
            continue;
            // return NULL;
        }
        if (SS[index].is_active == 0)
        {
            if (Client_packet.operation_code == READ)
            {
                int ret_index = -1;
                sem_wait(&mutex);
                if (SS[index].is_active == 0)
                {
                    if (SS[index].backup_index1 == -1)
                    {
                        ret_index = -1;
                    }
                    else if (SS[SS[index].backup_index1].is_active == 0)
                    {
                        if (SS[index].backup_index2 == -1)
                        {
                            ret_index = -1;
                        }
                        else if (SS[SS[index].backup_index2].is_active == 0)
                        {
                            printf("Server and backup server's crashed\n");
                            // sem_post(&mutex);
                            // exit(1);
                            ret_index = -1;
                        }
                        else
                        {
                            ret_index = SS[index].backup_index2;
                        }
                    }
                    else
                    {
                        ret_index = SS[index].backup_index1;
                    }
                }
                else
                {
                    ret_index = index;
                }
                index = ret_index;
                sem_post(&mutex);
                sem_wait(&mutex);
                ss_init temp_ss = SS[index];
                sem_post(&mutex);
                packet Return_packet;
                Return_packet.ns_port = temp_ss.client_port;
                Return_packet.operation_code = READ;
                strcpy(Return_packet.ns_ip_address, temp_ss.ip_address);
                // printf("a%sa", Return_packet.ns_ip_address);
                Return_packet.ack = 1;
                // printf("Hello\n");
                // printf("b%db", args->client_sock);

                int read_lock_ret = acquire_readlock(Client_packet.source_path_name);
                if (read_lock_ret == -1)
                {
                    packet SS_down;
                    SS_down.ack = 0;
                    SS_down.operation_code = TIME_OUT;
                    send(args->client_sock, (void *)&SS_down, sizeof(packet), 0);
                    add_to_log_file(CLIENT, "READ", 0, "Time Out", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                    continue;
                }
                else if (read_lock_ret == -2)
                {
                    packet SS_down;
                    SS_down.ack = 0;
                    SS_down.operation_code = NOT_FOUND;
                    send(args->client_sock, (void *)&SS_down, sizeof(packet), 0);
                    add_to_log_file(CLIENT, "READ", 0, "Not Found", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                    continue;
                }

                if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
                {
                    perror("Send error");
                    release_readlock(Client_packet.source_path_name);
                    exit(1);
                }
                printf("sent port for read\n");
                int ret = recv(args->client_sock, (void *)&Return_packet, sizeof(packet), 0);
                if (ret == -1)
                {
                    perror("Recv Error");
                    release_readlock(Client_packet.source_path_name);
                    // exit(1);
                    return NULL;
                }
                else if (ret == 0)
                {
                    printf("Client Disconnected\n");
                    release_readlock(Client_packet.source_path_name);
                    return NULL;
                }
                // printf("b%db", args->client_sock);

                fflush(stdout);
                if (Return_packet.operation_code == STOP)
                {
                    printf("Read oper done");
                }
                release_readlock(Client_packet.source_path_name);
            }
            else
            {
                packet SS_down;
                SS_down.ack = 0;
                SS_down.operation_code = NOT_FOUND;
                send(args->client_sock, (void *)&SS_down, sizeof(packet), 0);
            }
        }
        else if (Client_packet.operation_code == GET_INFO)
        {
            sem_wait(&mutex);
            ss_init temp_ss = SS[index];
            sem_post(&mutex);
            packet Return_packet;
            Return_packet.ns_port = temp_ss.client_port;
            strcpy(Return_packet.ns_ip_address, temp_ss.ip_address);
            Return_packet.ack = 1;
            Return_packet.operation_code = GET_INFO;
            // printf("Hello\n");
            if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send error");
                exit(1);
            }
            printf("sent port\n");
            int ret = recv(args->client_sock, (void *)&Return_packet, sizeof(packet), 0);
            if (ret == -1)
            {
                add_to_log_file(CLIENT, "GET_INFO", 0, "Recv Error", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                add_to_log_file(Storage_Server, "GET_INFO", 0, "Recv Error", NULL, temp_ss.ns_port, temp_ss.ip_address);
                perror("Recv Error");
                return NULL;
            }
            else if (ret == 0)
            {
                add_to_log_file(CLIENT, "GET_INFO", 0, "Client Disconnected", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                add_to_log_file(Storage_Server, "GET_INFO", 0, "Client Disconnected", NULL, temp_ss.ns_port, temp_ss.ip_address);
                printf("Client Disconnected\n");
                return NULL;
            }
            if (Return_packet.operation_code == STOP)
            {
                add_to_log_file(CLIENT, "GET_INFO", 1, "Success", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                add_to_log_file(Storage_Server, "GET_INFO", 1, "Success", NULL, temp_ss.ns_port, temp_ss.ip_address);
                printf("Read oper done");
            }
        }
        else if (Client_packet.operation_code == READ)
        {
            sem_wait(&mutex);
            ss_init temp_ss = SS[index];
            sem_post(&mutex);
            packet Return_packet;
            Return_packet.ns_port = temp_ss.client_port;
            Return_packet.operation_code = READ;
            strcpy(Return_packet.ns_ip_address, temp_ss.ip_address);
            // printf("a%sa", Return_packet.ns_ip_address);
            Return_packet.ack = 1;
            // printf("Hello\n");
            // printf("b%db", args->client_sock);

            int read_lock_ret = acquire_readlock(Client_packet.source_path_name);
            if (read_lock_ret == -1)
            {
                packet SS_down;
                SS_down.ack = 0;
                SS_down.operation_code = TIME_OUT;
                send(args->client_sock, (void *)&SS_down, sizeof(packet), 0);
                add_to_log_file(CLIENT, "READ", 0, "Time Out", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                continue;
            }
            else if (read_lock_ret == -2)
            {
                packet SS_down;
                SS_down.ack = 0;
                SS_down.operation_code = NOT_FOUND;
                send(args->client_sock, (void *)&SS_down, sizeof(packet), 0);
                add_to_log_file(CLIENT, "READ", 0, "Not Found", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                continue;
            }

            if (send(args->client_sock, (void *)&Return_packet, sizeof(packet), 0) == -1)
            {
                perror("Send error");
                release_readlock(Client_packet.source_path_name);
                exit(1);
            }
            printf("sent port for read\n");
            int ret = recv(args->client_sock, (void *)&Return_packet, sizeof(packet), 0);
            if (ret == -1)
            {
                perror("Recv Error");
                add_to_log_file(CLIENT, "READ", 0, "Recv Error", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                add_to_log_file(Storage_Server, "READ", 0, "Recv Error", NULL, temp_ss.ns_port, temp_ss.ip_address);
                release_readlock(Client_packet.source_path_name);
                // exit(1);
                return NULL;
            }
            else if (ret == 0)
            {
                printf("Client Disconnected\n");
                add_to_log_file(CLIENT, "READ", 0, "Client Disconnected", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                add_to_log_file(Storage_Server, "READ", 0, "Client Disconnected", NULL, temp_ss.ns_port, temp_ss.ip_address);
                release_readlock(Client_packet.source_path_name);
                return NULL;
            }
            // printf("b%db", args->client_sock);

            fflush(stdout);
            if (Return_packet.operation_code == STOP)
            {
                printf("Read oper done");
                add_to_log_file(CLIENT, "READ", 1, "Success", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                add_to_log_file(Storage_Server, "READ", 1, "Success", NULL, temp_ss.ns_port, temp_ss.ip_address);
            }
            release_readlock(Client_packet.source_path_name);
            // printf("This oper done\n");
        }
        else if (Client_packet.operation_code == LIST)
        {
            List Return_list;
            int *count = malloc(sizeof(int));
            char buff[256];
            strcpy(buff, Client_packet.source_path_name);
            // printf("Blaahh");
            fflush(stdout);
            char **paths = malloc(sizeof(char *) * MAX_PATHS);
            for (int i = 0; i < 100; i++)
                paths[i] = malloc(sizeof(char) * MAX_PATH_LENGTH);
            FindallwithPrefix(buff, ALL_PATHS, paths, count);
            printf("%d", *count);
            for (int i = 0; i < *count; i++)
            {
                strcpy(Return_list.paths[i], paths[i]);
                printf("%s", Return_list.paths[i]);
            }
            fflush(stdout);
            Return_list.npaths = *count;
            strcpy(Return_list.source_path_name, Client_packet.source_path_name);
            // printf("Hello\n");
            // printf("b%db", args->client_sock);
            if (send(args->client_sock, (void *)&Return_list, sizeof(List), 0) == -1)
            {
                perror("Send error");
                exit(1);
            }
            printf("sent list\n");
            packet Return_packet;
            // printf("b%db", args->client_sock);
            int ret = recv(args->client_sock, (void *)&Return_packet, sizeof(List), 0);
            if (ret == -1)
            {
                add_to_log_file(CLIENT, "LIST", 0, "Recv Error", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                perror("Recv Error");
                // exit(1);
                return NULL;
            }
            else if (ret == 0)
            {
                add_to_log_file(CLIENT, "LIST", 0, "Client Disconnected", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                printf("Client Disconnected\n");
                return NULL;
            }
            if (Return_packet.operation_code == STOP)
            {
                add_to_log_file(CLIENT, "LIST", 1, "Success", (struct sockaddr *)&(args->client_addr), -1, "Struct");
                printf("Read oper done");
            }
        }
        else
        {
            int stored_index = index;
            int backup_flag = 0;
            int backup_count = 0;
            for (int k = 0; k < 3; k++)
            {
                if (backup_count == 0)
                {
                    backup_count++;
                }
                else if (backup_count == 1)
                {
                    sem_wait(&mutex);
                    if (SS[SS[stored_index].backup_index1].is_active == 1)
                    {
                        index = SS[stored_index].backup_index1;
                        backup_flag = 1;
                        backup_count++;
                    }
                    else
                    {
                        backup_count++;
                        sem_post(&mutex);
                        continue;
                    }
                    sem_post(&mutex);
                }
                else if (backup_count == 2)
                {
                    sem_wait(&mutex);
                    if (SS[SS[stored_index].backup_index2].is_active == 1)
                    {
                        backup_flag = 1;
                        index = SS[stored_index].backup_index2;
                        backup_count++;
                    }
                    else
                    {
                        backup_count++;
                        sem_post(&mutex);
                        continue;
                    }
                    sem_post(&mutex);
                }
                exec_args *execute_args = malloc(sizeof(exec_args));
                execute_args->args = args;
                execute_args->Client_packet = Client_packet;
                execute_args->index = index;
                if (backup_flag == 1)
                {
                    execute_args->backup_flag = 1;
                }
                else
                {
                    execute_args->backup_flag = 0;
                }
                pthread_t *executing_thread = malloc(sizeof(pthread_t));
                pthread_create(executing_thread, NULL, executing, execute_args);
            }
        }
        // do something
        // send()

        // char command[1024];
        // scanf("%s", command);
        // rn assuming command is only path, just to add the trie vala part to find ss;

        // char *ip = "127.0.0.1";

        // int sock;
        // struct sockaddr_in addr;
        // socklen_t addr_size;
        // char buffer[1024];
        // int n;

        // sock = socket(AF_INET, SOCK_STREAM, 0);
        // if (sock < 0)
        // {
        //     perror("Socket error");
        //     exit(1);
        // }

        // memset(&addr, '\0', sizeof(addr));
        // addr.sin_family = AF_INET;
        // addr.sin_port = htons(port);
        // addr.sin_addr.s_addr = inet_addr(ip);
        // if (addr.sin_addr.s_addr == -1)
        // {
        //     perror("inet_addr error");
        //     exit(1);
        // }

        // if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        // {
        //     perror("Connect Error");
        //     exit(1);
        // }
        // // Recv message and send msg kya hoga ss se, abhi string rakha hai baadmai will add jo bhi struct chaiye rahega
        // bzero(buffer, 1024);
        // if (send(sock, buffer, strlen(buffer), 0) == -1)
        // {
        //     perror("Send error");
        //     exit(1);
        // }
        // bzero(buffer, 1024);
        // if (recv(sock, buffer, sizeof(buffer), 0) == -1)
        // {
        //     perror("Recv Error");
        //     exit(1);
        // }
        // // Figure out a way to return this value, back to client, maybe have a response struct created in the client thread,
        // // pass it along as well, and then check value in response thread, if the code is an error code then there was error.
        // if (close(sock) == -1)
        // {
        //     perror("Close error");
        //     exit(1);
        // }
    }
}

void *ss_ns_handling_thread(void *arg)
{
    int ss_sock = *(int *)arg;
    // printf("%d\n",ss_sock);
    ss_init ss;
    int val = recv(ss_sock, (void *)&ss, sizeof(ss), 0);
    if (val == -1)
    {
        perror("Recv Error");
        // exit(1);
        return NULL;
    }
    if (val == 0)
    {
        printf("SS discsonnected\n");
        return NULL;
    }
    // printf("%s\n",ss.paths[0]);
    // Assuming 201 is sent for an okay request
    int megaflag = 0;
    int index;
    if (ss.code == 201)
    {
        for (int i = 0; i < MAX_SS; i++)
        {
            if (SS[i].client_port == ss.client_port && SS[i].ns_port == ss.ns_port)
            {
                int not_same = 0;
                for (int i = 0; i < ss.npaths; i++)
                {
                    int not_found = 1;
                    for (int i = 0; i < MAX_PATHS; i++)
                    {
                        if (strcmp(ss.paths[i], SS[i].paths[i]) == 0)
                        {
                            not_found = 0;
                            break;
                        }
                    }
                    if (not_found == 1)
                    {
                        not_same = 1;
                        break;
                    }
                }

                if (not_same)
                {
                    break;
                }

                sem_wait(&mutex);
                ss.is_active = 1;
                SS[index].is_active = 1;
                sem_post(&mutex);
                index = i;
                printf("SS %d reconnected\n", i + 1);
                char buff[256];
                strcpy(buff, "201_Created");
                if (send(ss_sock, buff, strlen(buff), 0) == -1)
                {
                    perror("Send error");
                    // exit(1);
                    return NULL;
                }
                megaflag = 1;
                return NULL;
            }
        }
        // printf("hello\n");
        // int index;
        if (megaflag == 0)
        {
            ss.backup_index1 = -1;
            ss.backup_index2 = -1;
            ss.is_active = 1;
            sem_wait(&mutex);
            for (int i = 0; i < ss.npaths; i++)
            {
                InsertTrie(ss.paths[i], ALL_PATHS, SS_count);
            }
            SS[SS_count] = ss;
            index = SS_count;
            SS_count++;
            sem_post(&mutex);
            printf("SS %d sucessfully initialised\n", SS_count);

            add_to_log_file(Storage_Server, "Connection", 0, "Success", NULL, ss.ns_port, ss.ip_address);
            // index = SS_count;
            char buff[256];
            strcpy(buff, "201_Created");
            if (send(ss_sock, buff, strlen(buff), 0) == -1)
            {
                perror("Send error");
                // exit(1);
                return NULL;
            }
        }
        close(ss_sock);
        // find indices of storage server to store backup to
        sem_wait(&mutex);
        int count = 0, total_active = 0;
        for (int i = 0; i < SS_count; i++)
        {
            if (SS[i].is_active)
            {
                total_active++;
            }
        }
        if (total_active >= 3)
        {
            for (int j = 0; j < SS_count; j++)
            {
                count = 0;
                if (SS[j].backup_index2 != -1)
                {
                    continue;
                }
                if (SS[j].backup_index1 != -1)
                {
                    count = 1;
                }

                for (int i = 0; i < SS_count; i++)
                {
                    if (SS[(i + j + 1) % SS_count].is_active)
                    {
                        if (count == 0)
                        {
                            pthread_t backup_thread;
                            // int *backup_index = malloc(sizeof(int));
                            struct indices *INDEX = malloc(sizeof(struct indices));
                            SS[j].backup_index1 = (i + j + 1) % SS_count;
                            INDEX->index = j;
                            // declare backupindex;
                            INDEX->index_dest = (i + j + 1) % SS_count;
                            pthread_create(&backup_thread, NULL, backup_fxn, INDEX);
                            sem_post(&mutex);
                            sem_wait(&backup_lock);
                            pthread_join(backup_thread, NULL);
                            sem_post(&backup_lock);
                            sem_wait(&mutex);
                            count++;
                        }
                        else if (count == 1)
                        {
                            pthread_t backup_thread;
                            // int *backup_index = malloc(sizeof(int));
                            struct indices *INDEX = malloc(sizeof(struct indices));
                            SS[j].backup_index2 = (i + j + 1) % SS_count;
                            INDEX->index = j;
                            // declare backupindex;
                            INDEX->index_dest = (i + j + 1) % SS_count;
                            pthread_create(&backup_thread, NULL, backup_fxn, INDEX);
                            sem_post(&mutex);
                            sem_wait(&backup_lock);
                            pthread_join(backup_thread, NULL);
                            sem_post(&backup_lock);
                            sem_wait(&mutex);
                            break;
                        }
                    }
                }
            }
            sem_post(&mutex);
        }
        else
            sem_post(&mutex);
        // DECIDE INDEX and THEN BACKUP BOTH/ONE/OTHER pending

        // add recv here karooo
        int sock1;
        struct sockaddr_in addr1;
        socklen_t addr_size1;
        char buffer[1024];
        int n;

        sock1 = socket(AF_INET, SOCK_STREAM, 0);
        if (sock1 < 0)
        {
            perror("Socket error");
            // exit(1);
            // continue;
            return NULL;
        }

        memset(&addr1, '\0', sizeof(addr1));
        addr1.sin_family = AF_INET;
        addr1.sin_port = htons(ss.ns_port);
        addr1.sin_addr.s_addr = inet_addr(ss.ip_address);

        if (addr1.sin_addr.s_addr == -1)
        {
            perror("inet_addr error");
            // exit(1);
            // continue;
            return NULL;
        }
        // sleep(1);
        usleep(10000);
        if (connect(sock1, (struct sockaddr *)&addr1, sizeof(addr1)) == -1)
        {
            perror("Connect Error");
            printf("scam");
            fflush(stdout);
            // exit(1);
            // continue;
            return NULL;
        }
        // char buffing[256];
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock1, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
        packet ping_packet;
        ping_packet.operation_code = PING;
        if (send(sock1, &ping_packet, sizeof(packet), 0) == (-1))
        {
            perror("Send error");
            return NULL;
        }
        while (1)
        {
            int ret = recv(sock1, &ping_packet, sizeof(packet), 0);
            if (ret <= 0)
            {
                printf("SS %d disconnected\n", index + 1);
                SS[index].is_active = 0;
                // perror("Send error");
                // exit(1);
                // continue;
                // return NULL;
                // perror("Recv error");
                if (close(sock1) == -1)
                {
                    perror("Close Error");
                    // exit(1);
                    return NULL;
                }
                if (close(ss_sock) == -1)
                {
                    perror("Close Error");
                    // exit(1);
                    return NULL;
                }
                return NULL;
            }
            else
            {
                ping_packet.operation_code = PING;
                if (send(sock1, &ping_packet, sizeof(packet), 0) == (-1))
                {
                    perror("Send error");
                    return NULL;
                }
            }
        }
    }
    else
    {
        if (close(ss_sock) == -1)
        {
            perror("Close Error");
            // exit(1);
            return NULL;
        }
        printf("SS %d sucessfully \n", SS_count);
    }
}

void *client_handling_thread(void *arg)
{
    char *ip = "127.0.0.1";
    int server_sock_c, client_sock;
    struct sockaddr_in server_addr_c, client_addr;
    socklen_t addr_size_c;
    char buffer[1024];
    int n;

    server_sock_c = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock_c < 0)
    {
        perror("Socket error");
        // exit(1);
        return NULL;
    }

    memset(&server_addr_c, '\0', sizeof(server_addr_c));
    server_addr_c.sin_family = AF_INET;
    server_addr_c.sin_port = htons(NS_PORT_C);
    server_addr_c.sin_addr.s_addr = INADDR_ANY;
    if (server_addr_c.sin_addr.s_addr == -1)
    {
        perror("inet_addr error");
        // exit(1);
        return NULL;
    }

    n = bind(server_sock_c, (struct sockaddr *)&server_addr_c, sizeof(server_addr_c));
    if (n < 0)
    {
        perror("Bind error");
        // exit(1);
        return NULL;
    }

    if (listen(server_sock_c, 100) == -1)
    {
        perror("Listen error");
        // exit(1);
        return NULL;
    }
    while (1)
    {
        // ab idhar se do i create thread for each client, ya ye accept se data aya usse array mai store karte jao,
        // idk if these accept things will prevent one recvfrom from merging with another help
        struct arguments_client_thread *Arguments = malloc(sizeof(struct arguments_client_thread));
        client_sock = accept(server_sock_c, (struct sockaddr *)&client_addr, &addr_size_c);
        if (client_sock == -1)
        {
            perror("Accept error");
            // exit(1);
            continue;
        }
        printf("Client connected.\n");

        Arguments->addr_size_c = addr_size_c;
        Arguments->client_addr = client_addr;
        Arguments->client_sock = client_sock;
        pthread_t *client_connected_thread = malloc(sizeof(pthread_t));
        pthread_create(client_connected_thread, NULL, ns_ss_called_by_client, Arguments);
        // and then for each command recv we can create thread here and call ns-ss wala function
        // if (close(client_sock) == -1)
        // {
        //     perror("Close Error");
        //     exit(1);
        // }
    }
}

void *ss_handling_thread(void *arg)
{
    char *ip = "127.0.0.1";
    ss_init ss;
    int server_sock_ss, ss_sock;
    struct sockaddr_in server_addr_ss, ss_addr;
    socklen_t addr_size_ss;
    char buffer[1024];
    int n;

    server_sock_ss = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock_ss < 0)
    {
        perror("Socket error");
        // exit(1);
        return NULL;
    }

    memset(&server_addr_ss, '\0', sizeof(server_addr_ss));
    server_addr_ss.sin_family = AF_INET;
    server_addr_ss.sin_port = htons(NS_PORT_SS);
    server_addr_ss.sin_addr.s_addr = INADDR_ANY;
    if (server_addr_ss.sin_addr.s_addr == -1)
    {
        perror("inet_addr error");
        // exit(1);
        return NULL;
    }

    n = bind(server_sock_ss, (struct sockaddr *)&server_addr_ss, sizeof(server_addr_ss));
    if (n < 0)
    {
        perror("Bind error");
        // exit(1);
        return NULL;
    }

    if (listen(server_sock_ss, 100) == -1)
    {
        perror("Listen error");
        // exit(1);
        return NULL;
    }
    printf("Listening for ss's...\n");

    while (1)
    {
        ss_sock = accept(server_sock_ss, (struct sockaddr *)&ss_addr, &addr_size_ss);
        int *sock = malloc(sizeof(int));
        *sock = ss_sock;
        // printf("c%dc\n",ss_sock);
        if (ss_sock == -1)
        {
            perror("Accept error");
            // exit(1);
            continue;
        }
        printf("SS connected.\n");

        // printf("b%db\n",*sock);
        pthread_t *ss_thread = malloc(sizeof(pthread_t));
        pthread_create(ss_thread, NULL, ss_ns_handling_thread, sock);

        // if (close(ss_sock) == -1)
        // {
        //     perror("Close Error");
        //     exit(1);
        // }
        // printf("SS %d sucessfully initialised disconnected\n", SS_count);
    }
}

int main()
{
    init_cache();
    ALL_PATHS = InitTrie();
    sem_init(&mutex, 0, 1);
    sem_init(&backup_lock, 0, 1);
    pthread_t init_ss_connect;
    pthread_t init_client_connect;
    pthread_create(&init_client_connect, NULL, client_handling_thread, NULL);
    pthread_create(&init_ss_connect, NULL, ss_handling_thread, NULL);

    // Added a function to handle client requests, assuming rn we just have the path
    pthread_join(init_ss_connect, NULL);
    return 0;
}