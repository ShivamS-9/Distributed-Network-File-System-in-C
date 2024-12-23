#include "headers.h"

// int * nscon;
sem_t nslock;
// sem_t lock_write;

int copy_files(char *dir_target, int *con, packet *comm, char *path)
{
    DIR *dir;
    struct dirent *dp;
    dir = opendir(dir_target);
    if (!dir)
    {
        printf("\033[1;31mERROR: Couldn't open the directory\n");
        printf("\033[0m");
        return 0;
    }

    while (dp = readdir(dir))
    {
        struct stat stats;                                               // https://www.ibm.com/docs/en/i/7.2?topic=ssw_ibm_i_72/apis/stat.html
        char *stat_path = (char *)calloc(MAX_PATH_LENGTH, sizeof(char)); // https://codeforwin.org/c-programming/c-program-find-file-properties-using-stat-function
        char *dest_path = (char *)calloc(MAX_PATH_LENGTH, sizeof(char));
        strcat(stat_path, dir_target);
        if (strcmp(stat_path, "./") != 0)
            strcat(stat_path, "/");
        strcat(stat_path, dp->d_name);
        strcat(dest_path, path);
        if (strcmp(dest_path, "./") != 0)
            strcat(dest_path, "/");
        strcat(dest_path, dp->d_name);
        if (comm->operation_code == MOVE)
        {
            strcat(dest_path, "_copy");
        }

        if (!lstat(stat_path, &stats)) // https://linuxhint.com/lstat-function-c/
        {
            if (dp->d_name[0] != '.')
            {
                if (S_ISDIR(stats.st_mode))
                {
                    copy_files(stat_path, con, comm, dest_path);
                }
                else
                {
                    if (comm->operation_code != DELETE)
                    {
                        // if (!(dp->d_name[strlen(dp->d_name) - 1] == 't' && dp->d_name[strlen(dp->d_name) - 2] == 'u' && dp->d_name[strlen(dp->d_name) - 3] == 'o' && dp->d_name[strlen(dp->d_name) - 4] == '.'))
                        if (strstr(dp->d_name, ".out") == NULL)
                        {
                            if (comm->operation_code == BACKUP)
                            {
                                comm->operation_code = BACKUP;
                            }
                            else
                            {
                                comm->operation_code = MOVE;
                            }
                            comm->file_or_folder_code = FILE_TYPE;
                            for (int i = 0; i < MAX_PATH_LENGTH; i++)
                            {
                                comm->source_path_name[i] = '\0';
                                comm->destination_path_name[i] = '\0';
                            }
                            for (int i = 0; i < 4096; i++)
                            {
                                comm->contents[i] = '\0';
                            }
                            strcpy(comm->source_path_name, dir_target);
                            strcpy(comm->contents, dp->d_name);
                            strcpy(comm->destination_path_name, path);
                            if (send(*con, comm, sizeof(*comm), 0) == (-1))
                            {
                                // close(*con);
                                // free(con);
                                return 0;
                            }
                            usleep(5000);
                            // copy_files(stat_path,con,comm);
                        }
                    }
                    else
                    {
                        int ret = unlink(stat_path);
                        if (ret != 0)
                        {
                            return 0;
                        }
                    }
                }
            }
        }
        else
        {
            printf("\033[1;31mERROR: Couldn't get information about directory\n");
            printf("\033[0m");
            return 0;
        }
    }
    return 1;
}

int create_directory(char *dir_target, int *con, packet *comm, char *dire_path)
{
    DIR *dir;
    struct dirent *dp;
    fflush(stdout);
    dir = opendir(dir_target);
    if (!dir)
    {
        printf("\033[1;31mERROR: Couldn't open the directory\n");
        printf("\033[0m");
        return 0;
    }
    while (dp = readdir(dir))
    {
        struct stat stats;                                               // https://www.ibm.com/docs/en/i/7.2?topic=ssw_ibm_i_72/apis/stat.html
        char *stat_path = (char *)calloc(MAX_PATH_LENGTH, sizeof(char)); // https://codeforwin.org/c-programming/c-program-find-file-properties-using-stat-function
        strcat(stat_path, dir_target);
        if (strcmp(stat_path, "./") != 0)
        {
            strcat(stat_path, "/");
            // printf("Hello\n");
        }
        strcat(stat_path, dp->d_name);

        char *direct_path = (char *)calloc(MAX_PATH_LENGTH, sizeof(char));
        strcat(direct_path, dire_path);
        if (strcmp(direct_path, "./") != 0)
        {
            strcat(direct_path, "/");
            // printf("Hello\n");
        }
        strcat(direct_path, dp->d_name);
        if (comm->operation_code == MOVE)
        {
            strcat(direct_path, "_copy");
        }

        if (!lstat(stat_path, &stats)) // https://linuxhint.com/lstat-function-c/
        {
            if (dp->d_name[0] != '.')
            {
                if (S_ISDIR(stats.st_mode))
                {
                    if (comm->operation_code != DELETE)
                    {
                        int temp = comm->operation_code;
                        comm->file_or_folder_code = FOLDER_TYPE;
                        for (int i = 0; i < MAX_PATH_LENGTH; i++)
                        {
                            comm->source_path_name[i] = '\0';
                            comm->contents[i] = '\0';
                        }
                        strcat(comm->source_path_name, dire_path);
                        strcat(comm->contents, dp->d_name);
                        if (comm->operation_code == MOVE)
                        {
                            strcat(comm->contents, "_copy");
                        }
                        comm->operation_code = CREATE;
                        if (send(*con, comm, sizeof(*comm), 0) == (-1))
                        {
                            // close(*con);
                            // free(con);
                            return 0;
                        }
                        comm->operation_code = temp;
                        create_directory(stat_path, con, comm, direct_path);
                    }
                    else
                    {
                        create_directory(stat_path, con, comm, direct_path);
                        int ret = rmdir(stat_path);
                        if (ret != 0)
                        {
                            return 0;
                        }
                    }
                }
            }
        }
        else
        {
            printf("\033[1;31mERROR: Couldn't get information about directory\n");
            printf("\033[0m");
            return 0;
        }
    }
    return 1;
}

void *process_req(void *arg)
{
    cont_thread *info = (cont_thread *)arg;

    // printf(CYAN "[SS]: Starting Executing Request of Naming Server\n" DEFAULT);

    // Check For Functions
    if (info->command->operation_code == CREATE)
    {
        printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, info->command->operation_code);
        char pathname[MAX_PATH_LENGTH];
        for (int i = 0; i < MAX_PATH_LENGTH; i++)
            pathname[i] = '\0';
        strcpy(pathname, info->command->source_path_name);
        if (strcmp(pathname, "./") != 0)
            strcat(pathname, "/");
        strcat(pathname, info->command->contents);
        if (info->command->file_or_folder_code == FILE_TYPE)
        {
            int fd = open(pathname, O_CREAT | O_WRONLY | O_TRUNC, 0755);
            if (fd == (-1))
            {
                printf(RED "ERROR %d: File cannot be opened\n" DEFAULT, UNAUTHORIZED);
                fflush(stdout);
                info->command->operation_code = UNAUTHORIZED;
            }
            else
            {
                printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                info->command->operation_code = STOP;
            }
            close(fd);
            if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(*(info->conn));
                free((info->conn));
                free(info->command);
                free(info);
                return NULL;
            }
        }
        else if (info->command->file_or_folder_code == FOLDER_TYPE)
        {
            printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, info->command->operation_code);
            // printf("A%sA\n", pathname);
            int ret = mkdir(pathname, 0755);
            if (ret == (-1))
            {
                DIR *dir;
                // struct dirent *dp;
                dir = opendir(pathname);
                if (dir)
                {
                    printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                    info->command->operation_code = STOP;
                }
                else
                {
                    // printf("Compile hogaya\n");
                    printf(RED "ERROR %d: Folder cannot be opened\n" DEFAULT, UNAUTHORIZED);
                    info->command->operation_code = UNAUTHORIZED;
                }
            }
            else
            {
                printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                info->command->operation_code = STOP;
            }
            if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(*(info->conn));
                free((info->conn));
                free(info->command);
                free(info);
                return NULL;
            }
        }
    }
    else if (info->command->operation_code == DELETE)
    {
        printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, info->command->operation_code);
        char pathname[MAX_PATH_LENGTH];
        for (int i = 0; i < MAX_PATH_LENGTH; i++)
            pathname[i] = '\0';
        strcpy(pathname, info->command->source_path_name);
        if (strcmp(pathname, "./") != 0)
            strcat(pathname, "/");
        strcat(pathname, info->command->contents);
        if (info->command->file_or_folder_code == FOLDER_TYPE)
        {
            char rm_com[MAX_PATH_LENGTH];
            for (int i = 0; i < MAX_PATH_LENGTH; i++)
                rm_com[i] = '\0';
            strcpy(rm_com, "rm -r ");
            strcat(rm_com, pathname);
            // int status = system(rm_com);
            // if(status != 0)
            // {
            //     info->command->operation_code = UNAUTHORIZED;
            // }
            // else
            // {
            //     info->command->operation_code = STOP;
            // }
            if (copy_files(pathname, info->conn, info->command, info->command->destination_path_name) == 0)
            {
                info->command->operation_code = UNAUTHORIZED;
            }
            else
            {
                if (create_directory(pathname, info->conn, info->command, info->command->source_path_name) == 0)
                {
                    info->command->operation_code = UNAUTHORIZED;
                }
                else
                {
                    int ret = rmdir(pathname);
                    if (ret != 0)
                    {
                        printf(RED "ERROR %d: Folder Couldn't be Deleted\n" DEFAULT, UNAUTHORIZED);
                        info->command->operation_code = UNAUTHORIZED;
                    }
                    else
                    {
                        printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                        info->command->operation_code = STOP;
                    }
                }
            }
            if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(*(info->conn));
                free((info->conn));
                free(info->command);
                free(info);
                return NULL;
            }
        }
        else if (info->command->file_or_folder_code == FILE_TYPE)
        {
            int ret = unlink(pathname);
            if (ret != 0)
            {
                printf(RED "ERROR %d: File Couldn't be Deleted\n" DEFAULT, UNAUTHORIZED);
                info->command->operation_code = UNAUTHORIZED;
            }
            else
            {
                printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                info->command->operation_code = STOP;
            }
            if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(*(info->conn));
                free((info->conn));
                free(info->command);
                free(info);
                return NULL;
            }
        }
    }
    else if (info->command->operation_code == MOVE || info->command->operation_code == BACKUP)
    {
        // printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, info->command->operation_code);
        char pathname[MAX_PATH_LENGTH];
        for (int i = 0; i < MAX_PATH_LENGTH; i++)
            pathname[i] = '\0';
        strcpy(pathname, info->command->source_path_name);
        if (strcmp(pathname, "./") != 0)
            strcat(pathname, "/");
        strcat(pathname, info->command->contents);
        // info->command->ack = info->command->operation_code;

        if (info->command->file_or_folder_code == FILE_TYPE)
        {
            // Setting FileName
            if (info->command->operation_code == MOVE)
            {
                for (int i = 1; i < 5; i++)
                {
                    info->command->contents[strlen(info->command->contents) - 1] = '\0';
                }
                strcat(info->command->contents, "_copy.txt");
            }
            int fd = open(pathname, O_RDONLY);
            if (strcmp(info->command->destination_path_name, "./") != 0)
                strcat(info->command->destination_path_name, "/");
            strcat(info->command->destination_path_name, info->command->contents);
            if (fd == (-1))
            {
                // printf(RED "ERROR %d: File cannot be opened\n" DEFAULT, UNAUTHORIZED);
                info->command->operation_code = UNAUTHORIZED;
            }
            else
            {
                for (int i = 0; i < 4096; i++)
                    info->command->contents[i] = '\0';
                long long int total_data = lseek(fd, 0, SEEK_END);
                lseek(fd, 0, SEEK_SET);
                int overwrite_flag = 0;
                if (total_data == 0)
                {
                    int temp = read(fd, info->command->contents, sizeof(info->command->contents) - 1);
                    info->command->contents[4095] = '\0';
                    info->command->operation_code = APPEND;
                    if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
                    {
                        printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                        close(fd);
                        close(*(info->conn));
                        free((info->conn));
                        free(info->command);
                        free(info);
                        return NULL;
                    }
                }
                while (total_data > 0)
                {
                    int temp = read(fd, info->command->contents, sizeof(info->command->contents) - 1);
                    info->command->contents[4095] = '\0';
                    if (overwrite_flag == 0)
                    {
                        info->command->operation_code = WRITE;
                    }
                    else
                    {
                        info->command->operation_code = APPEND;
                    }
                    if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
                    {
                        printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                        close(fd);
                        close(*(info->conn));
                        free((info->conn));
                        free(info->command);
                        free(info);
                        return NULL;
                    }
                    // temp = lseek(fd,0,SEEK_CUR);
                    total_data -= temp;
                    for (int i = 0; i < 4096; i++)
                    {
                        info->command->contents[i] = '\0';
                    }
                    overwrite_flag = 1;
                    usleep(5000);
                }
                info->command->operation_code = STOP;
                close(fd);
            }
            if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(*(info->conn));
                free((info->conn));
                free(info->command);
                free(info);
                return NULL;
            }
        }
        else if (info->command->file_or_folder_code == FOLDER_TYPE)
        {
            printf(CYAN "Executing Command with Operation Code %d\n" DEFAULT, info->command->operation_code);
            char path[MAX_PATH_LENGTH];
            for (int i = 0; i < MAX_PATH_LENGTH; i++)
                path[i] = '\0';
            packet *comm = (packet *)malloc(sizeof(packet));
            for (int i = 0; i < MAX_PATH_LENGTH; i++)
            {
                comm->destination_path_name[i] = '\0';
                comm->source_path_name[i] = '\0';
            }
            for (int i = 0; i < 4096; i++)
                comm->contents[i] = '\0';
            strcpy(comm->source_path_name, info->command->destination_path_name);
            strcpy(comm->contents, info->command->contents);
            if (info->command->operation_code == MOVE)
            {
                strcat(comm->contents, "_copy");
            }
            strcpy(comm->destination_path_name, info->command->destination_path_name);
            if (strcmp(comm->destination_path_name, "./") != 0)
                strcat(comm->destination_path_name, "/");
            strcat(comm->destination_path_name, comm->contents);
            comm->file_or_folder_code = FOLDER_TYPE;
            if (info->command->operation_code == MOVE)
            {
                comm->operation_code = CREATE;
                if (send(*(info->conn), comm, sizeof(*(comm)), 0) == (-1))
                {
                    printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                    close(*(info->conn));
                    free((info->conn));
                    free(info->command);
                    free(info);
                    return NULL;
                }
            }
            comm->operation_code = info->command->operation_code;

            // strcat(comm->contents)
            // mkdir()
            if (create_directory(pathname, (info->conn), comm, comm->destination_path_name) == 0)
            {
                info->command->operation_code = UNAUTHORIZED;
            }
            else
            {
                int temp = info->command->operation_code;
                info->command->operation_code = STOP;
                if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
                {
                    printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                    close(*(info->conn));
                    free((info->conn));
                    free(info->command);
                    free(info);
                    return NULL;
                }
                info->command->operation_code = temp;
                for (int i = 0; i < MAX_PATH_LENGTH; i++)
                {
                    comm->source_path_name[i] = '\0';
                    comm->destination_path_name[i] = '\0';
                    comm->source_path_name[i] = '\0';
                }
                // info->command->operation_code = (-1);
                for (int i = 0; i < 4096; i++)
                    comm->contents[i] = '\0';
                char temp_path[MAX_PATH_LENGTH];
                strcpy(temp_path, info->command->destination_path_name);
                if (strcmp(temp_path, "./") != 0)
                    strcat(temp_path, "/");
                strcat(temp_path, info->command->contents);
                if (info->command->operation_code == MOVE)
                {
                    strcat(temp_path, "_copy");
                }
                if (copy_files(pathname, (info->conn), comm, temp_path) == 0)
                {
                    info->command->operation_code = UNAUTHORIZED;
                }
                else
                {
                    info->command->operation_code = STOP_CPY;
                    printf(CYAN "SUCCESS %d: Request Executed Successfully\n" DEFAULT, SUCCESS);
                }
            }
            if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
            {
                printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
                close(*(info->conn));
                free((info->conn));
                free(info->command);
                free(info);
                return NULL;
            }
        }
    }
    else if (info->command->operation_code == APPEND || info->command->operation_code == WRITE)
    {
        char pathname[MAX_PATH_LENGTH];
        for (int i = 0; i < MAX_PATH_LENGTH; i++)
            pathname[i] = '\0';
        strcpy(pathname, info->command->destination_path_name);
        // strcat(pathname,"/");
        // strcat(pathname,info->command->contents);
        int fd;
        if (info->command->operation_code == WRITE)
        {
            fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0755);
        }
        else
        {
            fd = open(pathname, O_WRONLY | O_CREAT, 0755);
        }
        if (fd == (-1))
        {
            printf(RED "ERROR %d: File cannot be opened\n" DEFAULT, UNAUTHORIZED);
            info->command->operation_code = UNAUTHORIZED;
        }
        else
        {
            // sem_wait(&lock_write);
            lseek(fd, 0, SEEK_END);
            if (write(fd, info->command->contents, strlen(info->command->contents)) == (-1))
            {
                printf(RED "ERROR %d: File Cannot be Written\n" DEFAULT, UNAUTHORIZED);
                info->command->operation_code = UNAUTHORIZED;
            }
            else
            {
                info->command->operation_code = (-2);
            }
            // info->command->operation_code= (-2);
            // CHECK FOR NM TO BE DECIDED
            // printf("Kanda");
            // fflush(stdout);
            // printf("Everything is fine");
            // fflush(stdout);
            // sem_post(&lock_write);
            fsync(fd);
            close(fd);
        }
        if (send(*(info->conn), info->command, sizeof(*(info->command)), 0) == (-1))
        {
            printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
            close(*(info->conn));
            free((info->conn));
            free(info->command);
            free(info);
            return NULL;
        }
    }
    else if (info->command->operation_code == PING)
    {
        packet ping_packet;
        ping_packet.operation_code = PING;
        usleep(10000);
        if (send(*(info->conn), &ping_packet, sizeof(packet), 0) == (-1))
        {
            printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
            close(*(info->conn));
            free((info->conn));
            free(info->command);
            free(info);
            return NULL;
        }
    }
    // close(*(info->conn));
    free((info->conn));
    free(info->command);
    free(info);
    return NULL;
}

void *ns_req(void *arg)
{
    printf(CYAN "[SS]: Thread Created For Request From Naming Server\n" DEFAULT);
    int *index = (int *)arg;
    packet *command = (packet *)malloc(sizeof(packet));
    while (1)
    {
        if (recv(*index, command, sizeof(*command), 0) <= 0)
        {
            printf("[SS]: Naming Server Closed The Connection\n");
            close(*index);
            free(index);
            return NULL;
        }
        // printf(CYAN "[SS]: Request Received from Naming Server\n" DEFAULT);
        cont_thread *temp = (cont_thread *)malloc(sizeof(cont_thread));
        temp->command = (packet *)malloc(sizeof(packet));
        temp->conn = (int *)malloc(sizeof(int));
        *(temp->command) = *command;
        *(temp->conn) = *index;
        pthread_t thread;
        pthread_create(&thread, NULL, &process_req, temp);
    }
    return NULL;
}

void *ns_direct_con(void *arg)
{
    int sfd_nserver, length;
    struct sockaddr_in serverAddress, client;
    printf("%d\n", port_ns);

    // creating a socket and verifying it.
    sfd_nserver = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd_nserver == -1)
    {
        printf(RED "ERROR %d: Socket not created\n" DEFAULT, INIT_ERROR);
        exit(0);
    }

    memset(&serverAddress, '\0', sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port_ns);

    int bind_fd = bind(sfd_nserver, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bind_fd == (-1))
    {
        printf(RED "ERROR %d: Couldn't bind socket\n" DEFAULT, INIT_ERROR);
        close(sfd_nserver);
        exit(0);
    }

    if ((listen(sfd_nserver, SOMAXCONN)) == (-1))
    {
        printf(RED "ERROR %d: Nothing recieved, listening failed\n" DEFAULT, INIT_ERROR);
        close(sfd_nserver);
        exit(0);
    }

    sem_init(&nslock, 0, 1);
    // sem_init(&lock_write, 0, 1);

    int count = 0;
    length = (sizeof(client));
    pthread_t *nsrequest = (pthread_t *)malloc(sizeof(pthread_t));
    int nscon;
    while (1)
    {
        sem_wait(&nslock);
        nscon = accept(sfd_nserver, (struct sockaddr *)&client, &length);
        if (nscon < 0)
        {
            printf(RED "ERROR %d: Couldn't establish connection\n" DEFAULT, BAD_REQUEST);
            sem_post(&nslock);
        }
        else
        {
            printf(CYAN "SUCCESS %d: Connection Established Successfully\n" DEFAULT, CONNECTION);
            count++;
            int *index = (int *)malloc(sizeof(int));
            *index = nscon;
            nsrequest = (pthread_t *)realloc(nsrequest, count * sizeof(pthread_t));
            // nscon = (int*)realloc(nscon,count*sizeof(int));
            sem_post(&nslock);
            pthread_create(&nsrequest[count - 1], NULL, &ns_req, index);
        }
    }
    for (int i = 0; i < count; i++)
    {
        pthread_join(nsrequest[i], NULL);
    }
    close(sfd_nserver);
    return NULL;
}