// #include "client.h"

// // Global Variables
// char server_ip_address[16]; // IP Address of Naming Server
// int naming_server_file_descriptor, storage_server_file_descriptor;
// struct sockaddr_in nm_server_address, storage_server_address;

// int send_packet_to_naming_server(packet *packet_to_send, packet *packet_to_receive)
// {
//     printf("[CLIENT] Sending Packet to Naming Server\n");
//     // Send Packet to Naming Server
//     printf("a%da\n", packet_to_send->operation_code);
//     if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
//     {
//         perror("[CLIENT] Send To Naming Server\n");
//     }

//     printf("[CLIENT] Packet Sent to Naming Server, Waiting for response\n");

//     // Receive Packet from Naming Server
//     if (recv(naming_server_file_descriptor, packet_to_receive, sizeof(*packet_to_receive), 0) < 0)
//     {
//         perror("[CLIENT] Receive from Naming Server\n");
//     }

//     printf("[CLIENT] Packet Received from Naming Server:\n");
//     if (packet_to_receive->operation_code == STOP)
//     {
//         return 1;
//     }
//     else
//     {
//         return 1;
//     }
// }

// void interface_with_storage_server(packet *packet_to_send, int operation_to_execute)
// {
//     storage_server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
//     if (storage_server_file_descriptor == -1)
//     {
//         printf("[CLIENT] SS Socket Creation Error!\n");
//         exit(0);
//     }

//     memset(&storage_server_address, '\0', sizeof(storage_server_address));

//     storage_server_address.sin_family = AF_INET;
//     storage_server_address.sin_addr.s_addr = inet_addr(packet_to_send->ns_ip_address);
//     storage_server_address.sin_port = htons(packet_to_send->ns_port);

//     printf("[CLIENT] Preparing to connect directly to Storage Server.\n");

//     int connection_status = connect(storage_server_file_descriptor, (struct sockaddr *)&storage_server_address, sizeof(storage_server_address));
//     if (connection_status == -1)
//     {
//         perror("[CLIENT] Connection Error!\n");
//         exit(0);
//     }
//     else
//     {
//         printf("[CLIENT] Connection with Storage Server Successful!\n");
//     }

//     printf("[CLIENT] Sending Packet to Storage Server\n");

//     if (send(storage_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
//     {
//         perror("[CLIENT] Send To Storage Server\n");
//     }

//     printf("[CLIENT] Packet Sent to Storage Server\n");
//     printf("[CLIENT] Waiting for response from Storage Server\n");

//     packet *packet_to_receive = (packet *)malloc(sizeof(packet));

//     // Assumed data < 4096 bytes

//     if (operation_to_execute == READ)
//     {
//         printf("%s contents:\n", packet_to_send->source_path_name);

//         // FILE *fp = fopen("output.txt", "a");
//         // if (fp == NULL)
//         // {
//         //     printf("[CLIENT] File not found!\n");
//         //     return;
//         // }

//         // printf("file pointer: %p\n", fp);

//         while (1)
//         {
//             // fseek(fp, 0, SEEK_END);
//             packet_to_receive->ack = 0;
//             packet_to_receive->operation_code = READ;

//             if (recv(storage_server_file_descriptor, packet_to_receive, sizeof(*packet_to_receive), 0) <= 0)
//             {
//                 perror("[CLIENT] Receive from Storage Server\n");
//             }

//             // printf("$%d$", packet_to_receive->operation_code);
//             if (packet_to_receive->operation_code != STOP)
//             {
//                 printf("%s", packet_to_receive->contents);
//                 fflush(stdout);
//                 // fprintf(fp, "%s", packet_to_receive->contents);
//                 // fflush(fp);
//             }
//             else
//             {
//                 printf("\n");
//                 printf("[CLIENT] Received complete file info.\n");
//                 break;
//             }
//         }

//         printf("[CLIENT] Read Operation Successful\n");
//     }
//     else if (operation_to_execute == WRITE)
//     {
//         printf("[CLIENT] Enter data to write to file: ");
//         char write_to_file_input[MAX_CONTENT];
//         char garb;
//         scanf("%c", &garb);
//         fgets(write_to_file_input, MAX_CONTENT, stdin);
//         write_to_file_input[strlen(write_to_file_input)] = '\0';
//         strcpy(packet_to_send->contents, write_to_file_input);

//         printf("[CLIENT] Sending Packet to Storage Server\n");

//         if (send(storage_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
//         {
//             perror("[CLIENT] Write Operation Error");
//         }

//         if (recv(storage_server_file_descriptor, packet_to_receive, sizeof(*packet_to_receive), 0) < 0)
//         {
//             perror("[CLIENT] Receive from Storage Server\n");
//         }

//         if (packet_to_receive->operation_code == STOP)
//         {
//             printf("[CLIENT] Write Operation Successful\n");
//         }
//     }
//     else if (operation_to_execute == GET_INFO)
//     {
//         char file_info[MAX_PATH_LENGTH];
//         for (int i = 0; i < MAX_PATH_LENGTH; i++)
//         {
//             file_info[i] = '\0';
//         }

//         if (recv(storage_server_file_descriptor, &file_info, MAX_PATH_LENGTH, 0) < 0)
//         {
//             perror("[CLIENT] Receive from Naming Server\n");
//         }

//         char *token = strtok(file_info, " ");

//         printf("File: %s\n", packet_to_send->source_path_name);
//         printf("File Permissions: %s\n", token);

//         token = strtok(NULL, " ");
//         printf("File Size: %s\n", token);
//     }

//     printf("[CLIENT] Sending Ack to NS.\n");

//     packet_to_send->ack = 1;
//     packet_to_send->operation_code = STOP;

//     if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
//     {
//         perror("[CLIENT] NS Operation Ack Error");
//     }

//     printf("[CLIENT] Closing Connection with Storage Server.\n");

//     close(storage_server_file_descriptor);

//     return;
// }

// void list_paths()
// {
//     packet *packet_to_send = (packet *)malloc(sizeof(packet));
//     packet_to_send->operation_code = LIST;
//     char choice = '\0';

//     printf("[CLIENT] Do you want to list all paths? (y/n): ");
//     getchar();
//     scanf("%c", &choice);

//     for (int i = 0; i < MAX_PATH_LENGTH; i++)
//     {
//         packet_to_send->source_path_name[i] = '\0';
//     }

//     if (choice == 'y')
//     {
//         packet_to_send->source_path_name[0] = '/';
//     }
//     else
//     {
//         printf("[CLIENT] Enter Path: ");
//         scanf("%s", packet_to_send->source_path_name);
//         packet_to_send->source_path_name[strlen(packet_to_send->source_path_name)] = '/';
//     }

//     if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
//     {
//         perror("[CLIENT] Send To Naming Server\n");
//     }

//     List *list = (List *)malloc(sizeof(List));

//     if (recv(naming_server_file_descriptor, list, sizeof(List), 0) < 0)
//     {
//         perror("[CLIENT] Receive from Naming Server\n");
//     }

//     printf("[CLIENT] Received paths from Naming Server\n");

//     for (int i = 0; i < list->npaths; i++)
//     {
//         printf("%s\n", list->paths[i]);
//     }

//     packet_to_send->ack = 1;
//     packet_to_send->operation_code = STOP;

//     if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
//     {
//         perror("[CLIENT] NS Operation Ack Error");
//     }

//     printf("[CLIENT] Sent Ack to NS.\n");

//     return;
// }

// // Code `1`: Read/Write File
// // Code `2`: Create/Delete File
// // Code `3`: Move File

// void access_file()
// {
//     // TO DO:
//     // 1. Send Message to NM
//     // 3. NM returns the precise IP address and client port for that SS to the client
//     // 4. Client connects to the SS and continuously receives information packets from the SS until a predefined “STOP” packet is sent
//     packet *packet_to_send = (packet *)malloc(sizeof(packet)), *packet_to_receive = (packet *)malloc(sizeof(packet));
//     char file_name[MAX_PATH_LENGTH];
//     int correct_input = 0;

//     // Get Operation Code
//     while (1)
//     {
//         // While loop to ensure that the user enters a valid input
//         printf("[CLIENT] Enter Operation Code (1: Read, 2: Write, 3: Get Info): ");
//         scanf("%u", &packet_to_send->operation_code);
//         if (packet_to_send->operation_code == 1 || packet_to_send->operation_code == 2 || packet_to_send->operation_code == 3)
//         {
//             break;
//         }
//         else
//         {
//             printf("[CLIENT] Invalid Input!\n");
//         }
//     }

//     if (packet_to_send->operation_code == 3)
//     {
//         packet_to_send->operation_code = GET_INFO;
//     }

//     packet_to_send->file_or_folder_code == FILE_TYPE;

//     // Get File/Folder Name
//     printf("[CLIENT] Enter File Name (w/path): ");
//     scanf("%s", file_name);
//     file_name[strlen(file_name)] = '\0';
//     strcpy(packet_to_send->source_path_name, file_name);

//     // Send Packet to Naming Server
//     while (send_packet_to_naming_server(packet_to_send, packet_to_receive) != 1)
//     {
//         // keep sending the packet until the operation is successful
//     }

//     if (packet_to_receive->operation_code == NOT_FOUND)
//     {
//         printf("[CLIENT] Invalid Path!\n");
//         return;
//     }

//     strcpy(packet_to_receive->source_path_name, file_name);
//     printf("[CLIENT] Packet Received from Naming Server\n");

//     interface_with_storage_server(packet_to_receive, packet_to_send->operation_code);

//     return;
// }

// void create_delete_function()
// {
//     // TO DO
//     // 1. Send Message to NM
//     // 2. After executing the command, the NM sends a message to the client indicating whether the operation was successful or not (ACK)
//     packet *packet_to_send = (packet *)malloc(sizeof(packet)), *packet_to_receive = (packet *)malloc(sizeof(packet));
//     int create_or_delete_choice, file_or_folder_choice, correct_input = 0;
//     char file_name[MAX_PATH_LENGTH];

//     while (!correct_input)
//     {
//         printf("[CLIENT] Enter Operation Code (1: Create, 2: Delete): ");
//         scanf(" %d", &create_or_delete_choice);
//         if (create_or_delete_choice == 1 || create_or_delete_choice == 2)
//         {
//             correct_input = 1;
//             // packet_to_send->operation_code = WRITE + create_or_delete_choice;
//             if (create_or_delete_choice == 1)
//             {
//                 packet_to_send->operation_code = CREATE;
//             }
//             else if (create_or_delete_choice == 2)
//             {
//                 packet_to_send->operation_code = DELETE;
//             }
//         }
//         else
//         {
//             printf("[CLIENT] Invalid Input!\n");
//         }
//     }

//     correct_input = 0;

//     // Get File or Folder
//     while (!correct_input)
//     {
//         if (create_or_delete_choice == 1)
//         {
//             printf("[CLIENT] Do you want to create a File (1) or Folder (2): ");
//         }
//         else if (create_or_delete_choice == 2)
//         {
//             printf("[CLIENT] Do you want to delete a File (1) or Folder (2): ");
//         }
//         scanf(" %u", &packet_to_send->file_or_folder_code);
//         if (packet_to_send->file_or_folder_code == 1 || packet_to_send->file_or_folder_code == 2)
//         {
//             correct_input = 1;
//         }
//         else
//         {
//             printf("[CLIENT] Invalid Input!\n");
//         }
//     }

//     // Get File/Folder Name
//     if (packet_to_send->file_or_folder_code == FILE_TYPE)
//     {
//         printf("[CLIENT] Enter File Name: ");
//         scanf(" %s", file_name);
//     }
//     else if (packet_to_send->file_or_folder_code == FOLDER_TYPE)
//     {
//         printf("[CLIENT] Enter Folder Name: ");
//         scanf(" %s", file_name);
//     }
//     file_name[strlen(file_name)] = '\0';
//     strcpy(packet_to_send->contents, file_name);

//     printf("[CLIENT] Enter Path: ");
//     scanf("%s", file_name);
//     // Assume that empty path -> `/`
//     file_name[strlen(file_name)] = '\0';
//     strcpy(packet_to_send->source_path_name, file_name);

//     // Send Packet to Naming Server
//     while (send_packet_to_naming_server(packet_to_send, packet_to_receive) != 1)
//     {
//         // keep sending the packet until the operation is successful
//     }

//     if (packet_to_receive->ack == 1)
//     {
//         printf("[CLIENT] Acknowledgment Received from Naming Server\n");
//         printf("[CLIENT] `%s - %s` Operation Successful!\n", packet_to_send->operation_code == CREATE ? "CREATE" : "DELETE", packet_to_send->source_path_name);
//     }

//     return;
// }

// void move_function()
// {
//     // TO DO
//     // 1. Send Message to NM
//     // 2. After executing the command, the NM sends a message to the client indicating whether the operation was successful or not (ACK)
//     packet *packet_to_send = (packet *)malloc(sizeof(packet)), *packet_to_receive = (packet *)malloc(sizeof(packet));
//     char file_name[MAX_PATH_LENGTH], destination_path[MAX_PATH_LENGTH];
//     packet_to_send->operation_code = MOVE;
//     printf("[CLIENT] Do you want to move a File (1) or Folder (2): ");
//     scanf("%u", &packet_to_send->file_or_folder_code);

//     // Get File/Folder Name
//     if (packet_to_send->file_or_folder_code == FILE_TYPE)
//     {
//         printf("[CLIENT] Enter File Name: ");
//         scanf("%s", file_name);
//     }
//     else if (packet_to_send->file_or_folder_code == FOLDER_TYPE)
//     {
//         printf("[CLIENT] Enter Folder Name: ");
//         scanf("%s", file_name);
//     }
//     else
//     {
//         printf("[CLIENT] Invalid Input!\n");
//         return;
//     }
//     strcpy(packet_to_send->contents, file_name);
//     packet_to_send->contents[strlen(packet_to_send->contents)] = '\0';

//     if (packet_to_send->file_or_folder_code == FILE_TYPE)
//     {
//         printf("[CLIENT] Enter path to file: ");
//     }
//     else
//     {
//         printf("[CLIENT] Enter path to folder: ");
//     }
//     scanf(" %s", packet_to_send->source_path_name);
//     packet_to_send->source_path_name[strlen(packet_to_send->source_path_name)] = '\0';

//     // Get Destination Path
//     printf("[CLIENT] Enter Destination Path: ");
//     scanf("%s", destination_path);
//     destination_path[strlen(destination_path)] = '\0';
//     strcpy(packet_to_send->destination_path_name, destination_path);

//     // Send Packet to Naming Server
//     while (send_packet_to_naming_server(packet_to_send, packet_to_receive) != 1)
//     {
//         // keep sending the packet until the operation is successful
//     }

//     printf("[CLIENT] Acknoeledgment Received from Naming Server\n");

//     if (packet_to_receive->ack == 1)
//     {
//         printf("[CLIENT] Moved %s to %s succesfully!\n", packet_to_send->source_path_name, packet_to_send->destination_path_name);
//     }

//     return;
// }

// int main()
// {
//     naming_server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
//     if (naming_server_file_descriptor == -1)
//     {
//         printf("[CLIENT] Socket Creation Error!\n");
//         exit(0);
//     }

//     memset(&nm_server_address, '\0', sizeof(nm_server_address));

//     // Get Server's IP Address by inputting 16 bit IP Address
//     // printf("[CLIENT] Enter Server's IP Address: ");
//     // scanf("%s", server_ip_address);
//     strcpy(server_ip_address, "127.0.0.1");
//     nm_server_address.sin_family = AF_INET;
//     nm_server_address.sin_addr.s_addr = inet_addr(server_ip_address); // Convert IP Address to Network Byte Order
//     nm_server_address.sin_port = htons(NAMING_SERVER_PORT);

//     printf("[CLIENT] Connecting to Naming Server...\n");

//     int connection_status = connect(naming_server_file_descriptor, (struct sockaddr *)&nm_server_address, sizeof(nm_server_address));
//     if (connection_status == -1)
//     {
//         printf("[CLIENT] Connection Error!\n");
//         exit(0);
//     }
//     else
//     {
//         printf("[CLIENT] Connection Successful!\n");
//     }

//     // Create a menu based system
//     // 1. Reading, Writing, and Retrieving Information about Files
//     // 2. Creating and Deleting Files and Folders
//     // 3. Copying Files/Directories Between Storage Servers
//     // 4. Exit
//     while (1)
//     {
//         int choice = 0;
//         printf("1. Reading, Writing, and Retrieving Information about Files\n");
//         printf("2. Creating and Deleting Files and Folders\n");
//         printf("3. Copying Files/Directories Between Storage Servers\n");
//         printf("4. List paths\n");
//         printf("5. Exit\n");
//         printf("Enter your choice: ");
//         scanf("%d", &choice);

//         if (choice == 1)
//         {
//             access_file();
//         }
//         else if (choice == 2)
//         {
//             create_delete_function();
//         }
//         else if (choice == 3)
//         {
//             move_function();
//         }
//         else if (choice == 4)
//         {
//             list_paths();
//         }
//         else if (choice == 5)
//         {
//             printf("[CLIENT] Exiting...\n");
//             close(naming_server_file_descriptor);
//             break;
//         }
//         else
//         {
//             printf("[CLIENT] Invalid Choice!\n");
//         }

//         fflush(stdin);
//     }

//     close(naming_server_file_descriptor);

//     return 0;
// }