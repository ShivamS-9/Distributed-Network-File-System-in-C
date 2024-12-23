#include "client.h"

void move_function()
{
    // TO DO
    // 1. Send Message to NM
    // 2. After executing the command, the NM sends a message to the client indicating whether the operation was successful or not (ACK)
    packet *packet_to_send = (packet *)malloc(sizeof(packet)), *packet_to_receive = (packet *)malloc(sizeof(packet));
    char file_name[MAX_PATH_LENGTH], destination_path[MAX_PATH_LENGTH];
    packet_to_send->operation_code = MOVE;
    printf(YELLOW "[CLIENT] Do you want to move a File (1) or Folder (2): " DEFAULT);
    scanf("%u", &packet_to_send->file_or_folder_code);

    // Get File/Folder Name
    if (packet_to_send->file_or_folder_code == FILE_TYPE)
    {
        printf(YELLOW "[CLIENT] Enter File Name: " DEFAULT);
        scanf("%s", file_name);
    }
    else if (packet_to_send->file_or_folder_code == FOLDER_TYPE)
    {
        printf(YELLOW "[CLIENT] Enter Folder Name: " DEFAULT);
        scanf("%s", file_name);
    }
    else
    {
        printf(RED "[CLIENT] Invalid Input!\n" DEFAULT);
        return;
    }
    strcpy(packet_to_send->contents, file_name);
    packet_to_send->contents[strlen(packet_to_send->contents)] = '\0';

    if (packet_to_send->file_or_folder_code == FILE_TYPE)
    {
        printf(YELLOW "[CLIENT] Enter path to file: " DEFAULT);
    }
    else
    {
        printf(YELLOW "[CLIENT] Enter path to folder: " DEFAULT);
    }
    scanf(" %s", packet_to_send->source_path_name);
    packet_to_send->source_path_name[strlen(packet_to_send->source_path_name)] = '\0';

    // Get Destination Path
    printf(YELLOW "[CLIENT] Enter Destination Path: " DEFAULT);
    scanf(" %s", destination_path);
    destination_path[strlen(destination_path)] = '\0';
    strcpy(packet_to_send->destination_path_name, destination_path);

    // Send Packet to Naming Server
    while (send_packet_to_naming_server(packet_to_send, packet_to_receive) != 1)
    {
        // keep sending the packet until the operation is successful
    }

    if (packet_to_receive->ack == 1)
    {
        printf(YELLOW "[CLIENT] Acknowledgment Received from Naming Server\n" DEFAULT);
        printf(YELLOW "[CLIENT] Moved %s to %s succesfully!\n" DEFAULT, packet_to_send->source_path_name, packet_to_send->destination_path_name);
    }
    else
    {
        printf(RED "[CLIENT] Error %d\n" DEFAULT, packet_to_receive->operation_code);
    }

    return;
}