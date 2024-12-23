#include "headers.h"

// int *con;
sem_t lock;

void *client_req(void *arg)
{
    int *index = (int *)arg;
    packet *command = (packet *)malloc(sizeof(struct packet));
    while (1)
    {
        if (recv(*index, command, sizeof(*command), 0) <= 0)
        {
            printf(CYAN "[SS]: Client Closed The Connection\n" DEFAULT);
            close(*index);
            free(index);
            return NULL;
        }
        printf(CYAN "[SS]: Request Received from Client\n" DEFAULT);
        printf(CYAN "[SS]: Starting Executing Request of Client\n" DEFAULT);
        // Check For Functions

        if (command->operation_code == READ)
        {
            printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, command->operation_code);
            int fd = open(command->source_path_name, O_RDONLY);
            if (fd == (-1))
            {
                printf(RED "ERROR %d: File cannot be opened\n" DEFAULT, UNAUTHORIZED);
                command->operation_code = UNAUTHORIZED;
            }
            else
            {
                // char buffer[4096];
                for (int i = 0; i < 4096; i++)
                {
                    command->contents[i] = '\0';
                }
                long long int total_data = lseek(fd, 0, SEEK_END);
                // printf("total data %lld\n", total_data);
                lseek(fd, 0, SEEK_SET);
                while (total_data > 0)
                {
                    // printf("%d\n", BUFFER_SIZE);
                    command->ack = 0;
                    int temp = read(fd, command->contents, sizeof(command->contents) - 1);
                    // if (temp == 0)
                    // {
                    //     continue;
                    // }
                    // printf("%d\n", temp);
                    command->contents[4095] = '\0';
                    // printf("p%ld\n", strlen(command->contents));
                    // printf(RED "%d\n" DEFAULT, command->operation_code);
                    // if (total_data < 4096)
                    // {
                    //     printf(CYAN "%s\n" DEFAULT, command->contents);
                    // }

                    if (send(*index, command, sizeof(*command), 0) == (-1))
                    {
                        printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                        close(*index);
                        free(index);
                        return NULL;
                    }
                    usleep(1000);
                    // temp = lseek(fd, 0, SEEK_CUR);
                    total_data -= temp;
                    for (int i = 0; i < 4096; i++)
                        command->contents[i] = '\0';
                }
                // for (int i = 0; i < 4096; i++)
                //     command->contents[i] = '\0';
                // if (send(*index, command, sizeof(*command), 0) == (-1))
                // {
                //     close(fd);
                //     close(*index);
                //     free(index);
                //     return NULL;
                // }
                command->operation_code = STOP;
                printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                command->ack = 1;
                close(fd);
            }
            if (send(*index, command, sizeof(*command), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(fd);
                close(*index);
                free(index);
                return NULL;
            }
        }
        else if (command->operation_code == WRITE)
        {
            // printf("%s\n", command->source_path_name);
            printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, command->operation_code);
            int fd = open(command->source_path_name, O_WRONLY | O_TRUNC);
            if (fd == (-1))
            {
                printf(RED "ERROR %d: File cannot be opened\n" DEFAULT, UNAUTHORIZED);
                command->operation_code = UNAUTHORIZED;
            }
            else
            {
                if (write(fd, command->contents, strlen(command->contents)) == (-1))
                {
                    printf(RED "ERROR %d: File cannot be written\n" DEFAULT, UNAUTHORIZED);
                    command->operation_code = UNAUTHORIZED;
                }
                else
                {
                    // printf("[SS]: File Writing Done, Sending ACK\n");
                    printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                    command->operation_code = STOP;
                }
            }
            if (send(*index, command, sizeof(*command), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(fd);
                close(*index);
                free(index);
                return NULL;
            }
            close(fd);
        }
        else if (command->operation_code == GET_INFO)
        {
            printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, command->operation_code);
            struct stat stats;
            char perm[1024];
            for (int i = 0; i < 1024; i++)
                perm[i] = '\0';
            if (!lstat(command->source_path_name, &stats))
            {
                char *typ;
                if (S_ISDIR(stats.st_mode)) // https://stackoverflow.com/questions/4989431/how-to-use-s-isreg-and-s-isdir-posix-macros
                {
                    typ = "d";
                }
                else if ((stats.st_mode & S_IXUSR))
                {
                    typ = "-";
                }
                else
                {
                    typ = "-";
                }
                char *temp;

                strcat(perm, typ);
                if (stats.st_mode & S_IRUSR)
                {
                    temp = "r";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IWUSR)
                {
                    temp = "w";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IXUSR)
                {
                    temp = "x";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IRGRP)
                {
                    temp = "r";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IWGRP)
                {
                    temp = "w";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IXGRP)
                {
                    temp = "x";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IROTH)
                {
                    temp = "r";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IWOTH)
                {
                    temp = "w";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);
                if (stats.st_mode & S_IXOTH)
                {
                    temp = "x";
                }
                else
                {
                    temp = "-";
                }
                strcat(perm, temp);

                strcat(perm, " ");

                char temp_1[256];
                for (int i = 0; i < 256; i++)
                    temp_1[i] = '\0';
                sprintf(temp_1, "%ld", stats.st_size);
                strcat(perm, temp_1);

                if (send(*index, perm, sizeof(perm), 0) == (-1))
                {
                    printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                    close(*index);
                    free(index);
                    return NULL;
                }
                else
                {
                    printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                    command->operation_code = STOP;
                }
            }
            else
            {
                printf(RED "ERROR %d: File cannot be opened\n" DEFAULT, UNAUTHORIZED);
                command->operation_code = UNAUTHORIZED;
            }
            if (send(*index, command, sizeof(*command), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(*index);
                free(index);
                return NULL;
            }
        }
    }
    return NULL;
}

void *client_direct_con(void *arg)
{
    int sfd_client, length;
    struct sockaddr_in serverAddress, client;
    printf("%d\n", port_client);

    // creating a socket and verifying it.
    sfd_client = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd_client == -1)
    {
        printf(RED "ERROR %d: Socket not created\n" DEFAULT, INIT_ERROR);
        exit(0);
    }

    memset(&serverAddress, '\0', sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port_client);

    int bind_fd = bind(sfd_client, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bind_fd == (-1))
    {
        printf(RED "ERROR %d: Couldn't bind socket\n" DEFAULT, INIT_ERROR);
        close(sfd_client);
        exit(0);
    }

    if ((listen(sfd_client, SOMAXCONN)) == (-1))
    {
        printf(RED "ERROR %d: Nothing recieved, listening failed\n" DEFAULT, INIT_ERROR);
        close(sfd_client);
        exit(0);
    }

    sem_init(&lock, 0, 1);

    int count = 0;
    length = (sizeof(client));
    pthread_t *request = (pthread_t *)malloc(sizeof(pthread_t));
    int con;
    while (1)
    {
        sem_wait(&lock);
        con = accept(sfd_client, (struct sockaddr *)&client, &length);
        if (con < 0)
        {
            printf(RED "ERROR %d: Couldn't establish connection\n" DEFAULT, BAD_REQUEST);
            sem_post(&lock);
        }
        else
        {
            printf(CYAN "SUCCESS %d: Connection Established Successfully\n" DEFAULT, CONNECTION);
            count++;
            int *index = (int *)malloc(sizeof(int));
            *index = con;
            request = (pthread_t *)realloc(request, count * (sizeof(pthread_t)));
            // con = (int *)realloc(con, count * (sizeof(int)));
            sem_post(&lock);
            pthread_create(&request[count - 1], NULL, &client_req, index);
        }
    }
    for (int i = 0; i < count; i++)
    {
        pthread_join(request[i], NULL);
    }
    close(sfd_client);
    return NULL;
}