#include "client.h"

int send_packet_to_naming_server(packet *packet_to_send, packet *packet_to_receive)
{
    printf(YELLOW "[CLIENT] Sending Packet to Naming Server\n" DEFAULT);
    // Send Packet to Naming Server
    // printf("a%da\n", packet_to_send->operation_code);
    if (send(naming_server_file_descriptor, packet_to_send, sizeof(packet), 0) < 0)
    {
        perror(RED "[CLIENT] Send To Naming Server\n" DEFAULT);
    }

    printf(YELLOW "[CLIENT] Packet Sent to Naming Server, Waiting for response\n" DEFAULT);

    // Receive Packet from Naming Server
    if (recv(naming_server_file_descriptor, packet_to_receive, sizeof(*packet_to_receive), 0) < 0)
    {
        perror(RED "[CLIENT] Receive from Naming Server\n" DEFAULT);
    }

    // printf(YELLOW "[CLIENT] Packet Received from Naming Server\n" DEFAULT);
    return 1;
}