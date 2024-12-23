# Distributed Network File System 

## Overview
This project implements a simple Network File System (NFS) from scratch. NFS allows multiple clients to interact with storage servers through a naming server that coordinates file access. It supports file operations such as creating, reading, writing, deleting, streaming, and copying files, with additional features like asynchronous and synchronous writes, error handling, and multiple client support.

## Key Components

### Naming Server (NM):
- Central hub that manages communication between clients and storage servers.
- Provides clients with the location of files and coordinates operations like file creation, deletion, and access.

### Storage Servers (SS):
- Stores the actual files and manages file operations such as reading, writing, and deletion.
- Can interact with multiple storage servers to manage and replicate data.

### Clients:
- The systems or users requesting file operations from the NFS.
- Interacts with the Naming Server to find the appropriate storage server and performs file operations directly with the Storage Server.

## Setup and Initialization

### Initialize Naming Server (NM):
- Start the Naming Server, which is the core of the NFS.
- Ensure the server listens for requests from both Storage Servers and Clients.

### Initialize Storage Servers (SS):
- Start one or more Storage Servers that will store files and provide access via the Naming Server.
- Each Storage Server registers itself with the Naming Server, providing details such as IP, port, and file access paths.

### Client Connection:
- Clients connect to the Naming Server to request file operations. The Naming Server returns the appropriate storage server’s details for further interaction.

## Features and Functionalities

### File Operations
- **Create File/Folder**: Clients can request to create new files or folders.
- **Read File**: Clients can retrieve the contents of a file.
- **Write File**: Clients can modify or create new content in a file.
- **Delete File/Folder**: Clients can delete files or folders.
- **List Files and Folders**: Clients can list all files and folders in a directory.
- **Get File Information**: Clients can retrieve metadata such as size, permissions, and timestamps.
- **Stream Audio Files**: Clients can stream audio files directly from the NFS.

### Asynchronous and Synchronous Writing
- **Asynchronous Write**: Clients can submit large files for writing without waiting for the entire operation to complete. The server responds immediately, and data is written to disk in chunks.
- **Synchronous Write**: A write operation that ensures the client waits for confirmation that the entire file has been written.

### Multiple Clients Support
- **Concurrency**: Multiple clients can interact with the system simultaneously. Reads are allowed concurrently, but writes are exclusive.

### Search and Caching
- **Efficient Search**: The Naming Server uses efficient search algorithms, such as Hashmaps or Tries, to locate storage servers.
- **LRU Caching**: The Naming Server implements an LRU caching strategy to speed up repeated access to recently requested data.

### Error Handling
- Custom error codes are returned when a file operation cannot be completed, for example, if a file does not exist or is being written to by another client.

### Backup and Replication
- **Failure Detection**: The Naming Server detects storage server failures and ensures data availability.
- **Data Replication**: Files are replicated across multiple storage servers to prevent data loss in case of failure.
