#include "headers.h"

int port_client;
int port_ns;
int port_univ;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        // Print an error message and exit the program
        fprintf(stderr, RED "Usage: %s <client-port> <ns-port>\n" DEFAULT, argv[0]);
        exit(EXIT_FAILURE);
    }

    // Assuming the rest of your code follows

    port_univ = 8080;
    port_client = atoi(argv[1]); // port for client-ss connection
    port_ns = atoi(argv[2]);     // port for ns-ss connection
    char *IP_addr;
    IP_addr = getIPaddr();

    int sfd_ns;
    struct sockaddr_in serverAddress;

    sfd_ns = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd_ns == -1)
    {
        printf(RED "ERROR %d: Socket not created\n" DEFAULT, INIT_ERROR);
        exit(0);
    }

    memset(&serverAddress, '\0', sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP_addr);
    serverAddress.sin_port = htons(port_univ);

    int con = connect(sfd_ns, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (con == (-1))
    {
        printf(RED "ERROR %d: Couldn't connect with server\n" DEFAULT, INIT_ERROR);
        close(sfd_ns);
        exit(0);
    }

    ns_init *pre = (ns_init *)malloc(sizeof(ns_init));

    int count = 0;
    char str[PATH_LEN];
    int counter = 0;
    for (int i = 0; i < PATH_LEN; i++)
        str[i] = '\0';
    int flag = 0;
    while (fgets(str, PATH_LEN, stdin))
    {
        if(flag == 0)
        {
            count = 0;
        }
        if (strcmp(str, "\n") == 0)
        {
            count++;
            flag = 1;
            if (count == 2)
            {
                break;
            }
            continue;
        }
        str[strlen(str) - 1] = '\0';
        strcpy(pre->paths[counter], str);
        for (int i = 0; i < PATH_LEN; i++)
            str[i] = '\0';
        counter++;
        flag = 0;
    }

    // ALLOCATING PACKET FOR INITIALIZATION

    for (int i = 0; i < strlen(IP_addr); i++)
        pre->ip_address[i] = IP_addr[i];
    pre->ip_address[strlen(IP_addr)] = '\0';
    pre->client_port = port_client;
    pre->ns_port = port_ns;
    pre->npaths = counter;
    pre->code = SUCCESS;
    printf("%d\n", pre->code);

    // INITIALIZATION PROCESS

    int send_sig = send(sfd_ns, (ns_init *)&(*pre), sizeof(*pre), 0);
    if (send_sig == (-1))
    {
        printf(RED "ERROR %d: Couldn't Send Packet\n" DEFAULT, NOT_SENT);
        close(sfd_ns);
        exit(0);
    }

    int recv_data = recv(sfd_ns, str, PATH_LEN, 0);
    if (recv_data <= 0)
    {
        printf(RED "ERROR %d: No Content\n" DEFAULT, EMPTY_PACKET);
        close(sfd_ns);
        exit(0);
    }
    // for(int i=0;i<PATH_LEN;i++)
    //     str[i]='\0';
    if (strcmp(str, "201_Created") != 0)
    {
        printf(RED "ERROR %d: Unable to Initialize the Server\n" DEFAULT, FAILED);
        close(sfd_ns);
        exit(0);
    }
    else
    {
        printf(CYAN "SUCCESS %d: Intialized Successfully\n" DEFAULT, INITIALIZATION);
    }

    pthread_t client;
    pthread_t naming_s;

    pthread_create(&client, NULL, &client_direct_con, NULL);
    pthread_create(&naming_s, NULL, &ns_direct_con, NULL);
    pthread_join(client, NULL);
    pthread_join(naming_s, NULL);

    close(sfd_ns);
}