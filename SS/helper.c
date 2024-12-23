#include "headers.h"

char * getIPaddr ()
{
    char* ip_address = (char*)calloc(256,sizeof(char));
    struct hostent *host_entry;
    char hostbuffer[256];
    char *IPbuffer;
    int hostname;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if(hostname != 0)
    {
        printf("\033[1;31mERROR: Error in obtaining IP address\n");
        printf("\033[0m");
        exit(0);        
    }
    host_entry = gethostbyname(hostbuffer);
    if(host_entry == NULL)
    {
        printf("\033[1;31mERROR: Error in obtaining IP address\n");
        printf("\033[0m");
        exit(0);         
    }
    ip_address = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    return ip_address;
}