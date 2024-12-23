#include "client.h"

// Code `1`: Read/Write File
// Code `2`: Create/Delete File
// Code `3`: Move File

// Global Variables
char server_ip_address[16]; // IP Address of Naming Server
int naming_server_file_descriptor, storage_server_file_descriptor;
struct sockaddr_in nm_server_address, storage_server_address;

int main()
{
    naming_server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (naming_server_file_descriptor == -1)
    {
        printf(RED "[CLIENT] Socket Creation Error!\n" DEFAULT);
        exit(0);
    }

    memset(&nm_server_address, '\0', sizeof(nm_server_address));

    // Get Server's IP Address by inputting 16 bit IP Address
    // printf("[CLIENT] Enter Server's IP Address: ");
    // scanf("%s", server_ip_address);
    strcpy(server_ip_address, DEFAULT_IP);
    nm_server_address.sin_family = AF_INET;
    nm_server_address.sin_addr.s_addr = inet_addr(server_ip_address); // Convert IP Address to Network Byte Order
    nm_server_address.sin_port = htons(NAMING_SERVER_PORT);

    printf(YELLOW "[CLIENT] Connecting to Naming Server...\n" DEFAULT);

    int connection_status = connect(naming_server_file_descriptor, (struct sockaddr *)&nm_server_address, sizeof(nm_server_address));
    if (connection_status == -1)
    {
        perror(RED "[CLIENT] Connection Error!\n" DEFAULT);
        exit(0);
    }
    else
    {
        printf(YELLOW "[CLIENT] Connection Successful!\n" DEFAULT);
    }

    // Create a menu based system
    // 1. Reading, Writing, and Retrieving Information about Files
    // 2. Creating and Deleting Files and Folders
    // 3. Copying Files/Directories Between Storage Servers
    // 4. Exit
    while (1)
    {
        int choice = 0;
        printf("1. Reading, Writing, and Retrieving Information about Files\n");
        printf("2. Creating and Deleting Files and Folders\n");
        printf("3. Copying Files/Directories Between Storage Servers\n");
        printf("4. List paths\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice == 1)
        {
            access_file();
        }
        else if (choice == 2)
        {
            create_delete_function();
        }
        else if (choice == 3)
        {
            move_function();
        }
        else if (choice == 4)
        {
            list_paths();
        }
        else if (choice == 5)
        {
            printf(RED "[CLIENT] Exiting...\n" DEFAULT);
            close(naming_server_file_descriptor);
            break;
        }
        else
        {
            printf(RED "[CLIENT] Invalid Choice!\n" DEFAULT);
        }

        fflush(stdin);
    }

    close(naming_server_file_descriptor);

    return 0;
}