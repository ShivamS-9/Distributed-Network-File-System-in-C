#ifndef __PACKET_H
#define __PACKET_H
// #include "SS/headers.h"

#define MAX_PATH_LENGTH 256
#define MAX_CONTENT 4096
#define IP_ADDRESS_LENGTH 16
#define MAX_PATHS 100
#define DEFAULT_IP "127.0.0.1"

// Color Codes
#define DEFAULT "\033[1;0m" // Reset
#define YELLOW "\033[1;93m" // Client
#define CYAN "\033[1;96m"   // Storage Server
#define GREEN "\033[1;92m"  // Naming Server
#define RED "\033[1;31m"    // Error

enum operation
{
    STOP = (-1),
    READ = 1,
    WRITE = 2,
    CREATE = 3,
    DELETE = 4,
    MOVE = 5,
    GET_INFO = 6,
    APPEND = 7,
    LIST = 8,
    STOP_CPY = 9,
    BACKUP = 10,
    PING = 11,
};

enum file_or_folder
{
    FILE_TYPE = 1,
    FOLDER_TYPE = 2
};

enum error_code
{
    // SUCCESS CODES
    SUCCESS = 201,        // FOR successful execution of request
    CONNECTION = 201,     // For successful connection between client or server (any of the thre makes connection wiht one another)
    INITIALIZATION = 202, // For successful initialization of storage server on naming server

    // ERROR CODES
    INIT_ERROR = 454,       // Error for socket API, Bind API, Listen API, Connect API
    BAD_REQUEST = 400,      // Error for Acc API
    UNAUTHORIZED = 402,     // Permission for Read, Write and Execute Denied
    EMPTY_PACKET = 403,     // Error for recv API
    NOT_SENT = 405,         // Error for send API
    HOST_ERROR = 406,       // Error in getting IP
    DISALLOW_REQUEST = 410, // Request Operation code other than defined for that handler
    NOT_FOUND = 404,        // File Path Not Found
    FAILED = 501,           // Failure of server intialization
    TIME_OUT = 408,         // Time out error
};

// Log ke liye
enum client_or_ss
{
    CLIENT = 0,
    Storage_Server = 1
};

typedef struct ns
{
    int client_port;
    int ns_port;
    char ip_address[256];
    char paths[MAX_PATHS][MAX_PATH_LENGTH];
    int npaths; // NUMBER OF PATHS
    int backup_index1;
    int backup_index2;
    int is_active;
    int code;

} ns_init;

struct packet
{
    int ack;                                     // Specifies whether the packet is an ACK or not
    int operation_code;                          // Specifies the operation to be performed
    int file_or_folder_code;                     // Specifies whether the file is a file or a folder
    char source_path_name[MAX_PATH_LENGTH];      // Specifies the source file/folder name
    char destination_path_name[MAX_PATH_LENGTH]; // If the operation is move, then this field specifies the destination file/folder name
    char contents[MAX_CONTENT];                  // Contains content to write/read
    char ns_ip_address[IP_ADDRESS_LENGTH];       // Stores storage server ip address
    int ns_port;                                 // Stores storage server port
};

typedef struct packet packet;

typedef struct List
{
    int npaths;
    char paths[MAX_PATHS][MAX_PATH_LENGTH];
    char source_path_name[MAX_PATH_LENGTH];
} List;

#endif