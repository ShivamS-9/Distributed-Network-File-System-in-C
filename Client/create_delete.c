#include "client.h"

void create_delete_function()
{
    // TO DO
    // 1. Send Message to NM
    // 2. After executing the command, the NM sends a message to the client indicating whether the operation was successful or not (ACK)
    packet *packet_to_send = (packet *)malloc(sizeof(packet)), *packet_to_receive = (packet *)malloc(sizeof(packet));
    int create_or_delete_choice, file_or_folder_choice, correct_input = 0;
    char file_name[MAX_PATH_LENGTH];

    while (!correct_input)
    {
        printf(YELLOW "[CLIENT] Enter Operation Code (1: Create, 2: Delete): " DEFAULT);
        scanf(" %d", &create_or_delete_choice);
        if (create_or_delete_choice == 1 || create_or_delete_choice == 2)
        {
            correct_input = 1;
            // packet_to_send->operation_code = WRITE + create_or_delete_choice;
            if (create_or_delete_choice == 1)
            {
                packet_to_send->operation_code = CREATE;
            }
            else if (create_or_delete_choice == 2)
            {
                packet_to_send->operation_code = DELETE;
            }
        }
        else
        {
            printf(RED "[CLIENT] Invalid Input!\n" DEFAULT);
        }
    }

    correct_input = 0;

    // Get File or Folder
    while (!correct_input)
    {
        if (create_or_delete_choice == 1)
        {
            printf(YELLOW "[CLIENT] Do you want to create a File (1) or Folder (2): " DEFAULT);
        }
        else if (create_or_delete_choice == 2)
        {
            printf(YELLOW "[CLIENT] Do you want to delete a File (1) or Folder (2): " DEFAULT);
        }
        scanf(" %u", &packet_to_send->file_or_folder_code);
        if (packet_to_send->file_or_folder_code == 1 || packet_to_send->file_or_folder_code == 2)
        {
            correct_input = 1;
        }
        else
        {
            printf("[CLIENT] Invalid Input!\n");
        }
    }

    // Get File/Folder Name
    if (packet_to_send->file_or_folder_code == FILE_TYPE)
    {
        printf(YELLOW "[CLIENT] Enter File Name: " DEFAULT);
        scanf(" %s", file_name);
    }
    else if (packet_to_send->file_or_folder_code == FOLDER_TYPE)
    {
        printf(YELLOW "[CLIENT] Enter Folder Name: " DEFAULT);
        scanf(" %s", file_name);
    }
    file_name[strlen(file_name)] = '\0';
    strcpy(packet_to_send->contents, file_name);

    printf(YELLOW "[CLIENT] Enter Path: " DEFAULT);
    scanf("%s", file_name);
    // Assume that empty path -> `/`
    file_name[strlen(file_name)] = '\0';
    strcpy(packet_to_send->source_path_name, file_name);

    // Send Packet to Naming Server
    while (send_packet_to_naming_server(packet_to_send, packet_to_receive) != 1)
    {
        // keep sending the packet until the operation is successful
    }

    if (packet_to_receive->ack == 1 || packet_to_receive->operation_code == STOP)
    {
        printf(YELLOW "[CLIENT] Acknowledgment Received from Naming Server\n" DEFAULT);
        printf(GREEN "[CLIENT] %s Operation Successful!\n" DEFAULT, packet_to_send->operation_code == CREATE ? "CREATE" : "DELETE");
    }
    else if (packet_to_receive->operation_code == UNAUTHORIZED)
    {
        printf(RED "[CLIENT] `%s - %s` Operation Failed! Permission Denied!\n" DEFAULT, packet_to_send->operation_code == CREATE ? "CREATE" : "DELETE", packet_to_send->source_path_name);
    }
    else
    {
        printf(RED "[CLIENT] Error %d\n" DEFAULT, packet_to_receive->operation_code);
        // printf(RED "[CLIENT] `%s - %s` Operation Failed!\n" DEFAULT, packet_to_send->operation_code == CREATE ? "CREATE" : "DELETE", packet_to_send->source_path_name);
    }

    return;
}