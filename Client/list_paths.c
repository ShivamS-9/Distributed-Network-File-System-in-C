#include "client.h"

void list_paths()
{
    packet *packet_to_send = (packet *)malloc(sizeof(packet));
    packet_to_send->operation_code = LIST;
    char choice = '\0';

    // printf(YELLOW "[CLIENT] Do you want to list all paths? (y/n): " DEFAULT);
    // getchar();
    // scanf("%c", &choice);

    for (int i = 0; i < MAX_PATH_LENGTH; i++)
    {
        packet_to_send->source_path_name[i] = '\0';
    }

    printf(YELLOW "[CLIENT] Enter Path: " DEFAULT);
    scanf("%s", packet_to_send->source_path_name);
    packet_to_send->source_path_name[strlen(packet_to_send->source_path_name)] = '/';

    if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
    {
        perror(RED "[CLIENT] Send To Naming Server\n" DEFAULT);
    }

    List *list = (List *)malloc(sizeof(List));

    if (recv(naming_server_file_descriptor, list, sizeof(List), 0) < 0)
    {
        perror(RED "[CLIENT] Receive from Naming Server\n" DEFAULT);
    }

    if (list->npaths == 0)
    {
        printf(RED "[CLIENT] No paths found!\n" DEFAULT);
        return;
    }

    printf(YELLOW "[CLIENT] Received paths from Naming Server\n" DEFAULT);

    for (int i = 0; i < list->npaths; i++)
    {
        printf("%s\n", list->paths[i]);
    }

    packet_to_send->ack = 1;
    packet_to_send->operation_code = STOP;

    if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
    {
        perror(RED "[CLIENT] NS Operation Ack Error" DEFAULT);
    }

    printf(YELLOW "[CLIENT] Sent Ack to NS.\n" DEFAULT);

    return;
}