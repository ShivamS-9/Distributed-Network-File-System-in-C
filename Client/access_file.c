#include "client.h"

void interface_with_storage_server(packet *packet_to_send, int operation_to_execute, char *string_to_write)
{
    storage_server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (storage_server_file_descriptor == -1)
    {
        perror(RED "[CLIENT] SS Socket Creation Error!\n" DEFAULT);
        return;
    }

    memset(&storage_server_address, '\0', sizeof(storage_server_address));

    storage_server_address.sin_family = AF_INET;
    storage_server_address.sin_addr.s_addr = inet_addr(packet_to_send->ns_ip_address);
    storage_server_address.sin_port = htons(packet_to_send->ns_port);

    printf(YELLOW "[CLIENT] Preparing to connect directly to Storage Server.\n" DEFAULT);

    int connection_status = connect(storage_server_file_descriptor, (struct sockaddr *)&storage_server_address, sizeof(storage_server_address));
    if (connection_status == -1)
    {
        perror(RED "[CLIENT] Connection Error!\n" DEFAULT);
        return;
    }
    else
    {
        printf(YELLOW "[CLIENT] Connection with Storage Server Successful!\n" DEFAULT);
    }

    printf(YELLOW "[CLIENT] Sending Packet to Storage Server\n" DEFAULT);

    if (send(storage_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
    {
        perror(RED "[CLIENT] Send To Storage Server\n" DEFAULT);
        close(storage_server_file_descriptor);
        return;
    }

    printf(YELLOW "[CLIENT] Packet Sent to Storage Server\n" DEFAULT);
    printf(YELLOW "[CLIENT] Waiting for response from Storage Server\n" DEFAULT);

    packet *packet_to_receive = (packet *)malloc(sizeof(packet));

    if (operation_to_execute == READ)
    {
        printf(CYAN "%s contents:\n" DEFAULT, packet_to_send->source_path_name);

        // FILE *fp = fopen("output.txt", "a");
        // if (fp == NULL)
        // {
        //     printf("[CLIENT] File not found!\n");
        //     return;
        // }

        // printf("file pointer: %p\n", fp);

        while (1)
        {
            // fseek(fp, 0, SEEK_END);
            packet_to_receive->ack = 0;
            packet_to_receive->operation_code = READ;

            if (recv(storage_server_file_descriptor, packet_to_receive, sizeof(*packet_to_receive), 0) <= 0)
            {
                perror(RED "[CLIENT] Receive from Storage Server\n" DEFAULT);
                close(storage_server_file_descriptor);
                return;
            }

            // printf("$%d$", packet_to_receive->operation_code);
            if (packet_to_receive->operation_code != STOP && packet_to_receive->operation_code != UNAUTHORIZED)
            {
                printf("%s", packet_to_receive->contents);
                fflush(stdout);
                // fprintf(fp, "%s", packet_to_receive->contents);
                // fflush(fp);
            }
            else
            {
                if (packet_to_receive->operation_code == STOP || packet_to_receive->operation_code == 0)
                {
                    printf("\n");
                    printf(YELLOW "[CLIENT] Received complete file info.\n" DEFAULT);
                    fflush(stdout);
                    close(storage_server_file_descriptor);
                    break;
                }
                else if (packet_to_receive->operation_code == UNAUTHORIZED)
                {
                    printf(RED "[CLIENT] Error %d : Unauthorised Access\n" DEFAULT, packet_to_receive->operation_code);
                    fflush(stdout);
                    close(storage_server_file_descriptor);
                    break;
                }
                else if (packet_to_receive->operation_code == NOT_FOUND)
                {
                    printf(RED "[CLIENT] Error %d : Invalid Path!\n" DEFAULT, packet_to_receive->operation_code);
                    fflush(stdout);
                    close(storage_server_file_descriptor);
                    break;
                }
                printf(RED "[CLIENT] Error %d in Read Operation\n" DEFAULT, packet_to_receive->operation_code);
                close(storage_server_file_descriptor);
                return;
            }
        }

        printf(GREEN "[CLIENT] Read Operation Successful\n" DEFAULT);
    }
    else if (operation_to_execute == WRITE)
    {
        // printf(YELLOW "[CLIENT] Enter data to write to file: " DEFAULT);
        // char write_to_file_input[MAX_CONTENT];
        // char garb;
        // scanf("%c", &garb);
        // fgets(write_to_file_input, MAX_CONTENT, stdin);
        strcpy(packet_to_send->contents, string_to_write);

        printf(YELLOW "[CLIENT] Sending Packet to Storage Server\n" DEFAULT);

        if (send(storage_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
        {
            perror(RED "[CLIENT] Write Operation Error" DEFAULT);
            close(storage_server_file_descriptor);
            return;
        }

        if (recv(storage_server_file_descriptor, packet_to_receive, sizeof(*packet_to_receive), 0) < 0)
        {
            perror(RED "[CLIENT] Receive from Storage Server\n" DEFAULT);
            close(storage_server_file_descriptor);
            return;
        }

        if (packet_to_receive->ack == 1)
        {
            printf(GREEN "[CLIENT] Write Operation Successful\n" DEFAULT);
        }
        else if (packet_to_receive->operation_code == UNAUTHORIZED)
        {
            printf(RED "[CLIENT] Error %d : Unauthorised Access\n" DEFAULT, packet_to_receive->operation_code);
        }
        else if (packet_to_receive->operation_code == NOT_FOUND)
        {
            printf(RED "[CLIENT] Error %d : Invalid Path!\n" DEFAULT, packet_to_receive->operation_code);
        }
        else
        {
            printf(RED "[CLIENT] Error %d in Write Operation\n" DEFAULT, packet_to_receive->operation_code);
        }
    }
    else if (operation_to_execute == GET_INFO)
    {
        char file_info[MAX_PATH_LENGTH];
        for (int i = 0; i < MAX_PATH_LENGTH; i++)
        {
            file_info[i] = '\0';
        }

        if (recv(storage_server_file_descriptor, &file_info, MAX_PATH_LENGTH, 0) < 0)
        {
            perror(RED "[CLIENT] Receive from Naming Server\n" DEFAULT);
        }

        char *token = strtok(file_info, " ");

        printf(CYAN "File       : \033[1;92m %s\n" DEFAULT, packet_to_send->source_path_name);
        printf(CYAN "Permissions: \033[1;92m %s\n" DEFAULT, token);

        token = strtok(NULL, " ");
        printf(CYAN "Size       : \033[1;92m %s\n" DEFAULT, token);
    }

    printf(YELLOW "[CLIENT] Sending Ack to NS.\n" DEFAULT);

    packet_to_send->ack = 1;
    packet_to_send->operation_code = STOP;

    if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
    {
        perror(RED "[CLIENT] NS Operation Ack Error" DEFAULT);
    }

    printf(YELLOW "[CLIENT] Closing Connection with Storage Server.\n" DEFAULT);

    close(storage_server_file_descriptor);

    return;
}

void access_file()
{
    // TO DO:
    // 1. Send Message to NM
    // 3. NM returns the precise IP address and client port for that SS to the client
    // 4. Client connects to the SS and continuously receives information packets from the SS until a predefined “STOP” packet is sent
    packet *packet_to_send = (packet *)malloc(sizeof(packet)), *packet_to_receive = (packet *)malloc(sizeof(packet));
    char file_name[MAX_PATH_LENGTH];
    int correct_input = 0;

    // Get Operation Code
    while (1)
    {
        // While loop to ensure that the user enters a valid input
        printf(YELLOW "[CLIENT] Enter Operation Code (1: Read, 2: Write, 3: Get Info): " DEFAULT);
        scanf("%u", &packet_to_send->operation_code);
        if (packet_to_send->operation_code == 1 || packet_to_send->operation_code == 2 || packet_to_send->operation_code == 3)
        {
            break;
        }
        else
        {
            printf(YELLOW "[CLIENT] Invalid Input!\n" DEFAULT);
        }
    }

    if (packet_to_send->operation_code == 3)
    {
        packet_to_send->operation_code = GET_INFO;
    }

    packet_to_send->file_or_folder_code == FILE_TYPE;

    // Get File/Folder Name
    printf(YELLOW "[CLIENT] Enter File Name (w/path): " DEFAULT);
    scanf("%s", file_name);
    file_name[strlen(file_name)] = '\0';
    strcpy(packet_to_send->source_path_name, file_name);

    char *write_to_file_input = (char *)calloc(MAX_CONTENT, sizeof(char));

    if (packet_to_send->operation_code == WRITE)
    {
        printf(YELLOW "[CLIENT] Enter data to write to file: " DEFAULT);
        getchar();
        fgets(write_to_file_input, MAX_CONTENT, stdin);
        write_to_file_input[strlen(write_to_file_input)] = '\0';
        strcpy(packet_to_send->contents, write_to_file_input);
    }

    // Send Packet to Naming Server
    while (send_packet_to_naming_server(packet_to_send, packet_to_receive) != 1)
    {
        // keep sending the packet until the operation is successful
    }

    if (packet_to_receive->operation_code == NOT_FOUND)
    {
        printf(RED "[CLIENT] Error %d : Invalid Path!\n" DEFAULT, packet_to_receive->operation_code);
        return;
    }
    else if (packet_to_receive->operation_code == TIME_OUT)
    {
        printf(RED "[CLIENT] Error %d : Operation Timed Out!\n" DEFAULT, packet_to_receive->operation_code);
        return;
    }

    else if (packet_to_receive->ack != 1)
    {
        printf(RED "[CLIENT] Error %d\n" DEFAULT, packet_to_receive->operation_code);
        return;
    }

    strcpy(packet_to_receive->source_path_name, file_name);
    printf(YELLOW "[CLIENT] Packet Received from Naming Server\n" DEFAULT);

    interface_with_storage_server(packet_to_receive, packet_to_send->operation_code, write_to_file_input);

    return;
}