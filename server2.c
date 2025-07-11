/**
 * Skeleton file for server.c
 * 
 * You are free to modify this file to implement the server specifications
 * as detailed in Assignment 3 handout.
 * 
 * As a matter of good programming habit, you should break up your imple-
 * mentation into functions. All these functions should contained in this
 * file as you are only allowed to submit this file.
 */ 

/**
 * NWEN 241 Assignment 2 (Challenge file)
 * marstetom - 300665781
 * All header comments are associated with their relevant task.
 * NOTE: This style of comments is ad-libbed from both Linux Kernel Coding Style & JAVADOC
 */

#include <stdio.h>
// Include necessary header files
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#define COMMAND_BUFFER_CAPACITY 256
#define FILE_NAME_BUFFER_CAPACITY 256
#define FILE_CONTENT_BUFFER_CAPACITY 512

/**
 * The main function should be able to accept a command-line argument
 * argv[0]: program name
 * argv[1]: port number
 * 
 * Read the assignment handout for more details about the server program
 * design specifications.
 */ 

/**
 * Helper
 * Sends an error message and then exits the proccess.
 * @param msg String of content of the error message.
 * @return void
 */
void error(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

/**
 * Task 1 - 2.
 * Opens the server socket.
 * @return void
 */
int open_socket() 
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        error("Error creating socket.");
    }
    printf("Socket successfully opened. (%d)\n", fd);
    return fd;
}

/**
 * Task 1 - 2.
 * Initialises a socket address (sockaddr_in) structure variable to relevant IP and port.
 * @param sockaddr_in Pointer to socket address variable
 * @param port Port number
 * @return void
 */
void create_socket(struct sockaddr_in *socket, in_port_t port) 
{
    socket->sin_family = AF_INET;
    socket->sin_port = htons(port);
    socket->sin_addr.s_addr = INADDR_ANY;
    printf("Address created.\n");
}

/**
 * Task 1 - 2.
 * Binds the socket address to the open server socket.
 * @param fd File descriptor of the server socket
 * @param sockaddr_in Pointer to socket variable
 * @return void
 */
void do_bind(int fd, struct sockaddr_in address)  
{
    int bd = bind(fd, (struct sockaddr *) &address, sizeof(address));
    if (bd == -1) {
        error("Error binding socket.");
    }
    printf("Socket %d successfully binded to address %d:%d.\n", fd, address.sin_addr.s_addr, address.sin_port);
}

/**
 * Task 1 - 3.
 * Listens for incoming connections from clients and then accepts them.
 * @param fd File descriptor of the server socket
 * @return Client file descriptor
 */
int listen_and_accept(int fd) 
{
    if (listen(fd, SOMAXCONN) < 0) {
        error("Error listening on socket."); 
    }
    printf("Socket successfully listening. (%d)\n", fd);
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(&client_addr);
    int client_fd = accept(fd, (struct sockaddr *) &client_addr, &addrlen);
    if (client_fd < 0) {
        error("Error accepting client socket.");
    }
    printf("Client socket accepted. (%d)\n", fd);
    return client_fd;
}

/**
 * Task 1 - 4.
 * Sends a message payload from the server to the receiving client.
 * @param client_fd File descriptor of the receiving client socket
 * @param msg String of message to be sent to client
 * @return void
 */
void send_to_client(int client_fd, char *msg) 
{   
    int w = write(client_fd, msg, strlen(msg));
    if (w < 0) {
        error("Error writing to socket.");
    }
    printf("Wrote message to the client: %s", msg);
}

/**
 * Task 1 - 5.
 * Awaits and reads any incoming string line from a sending client.
 * @param client_fd Client file descriptor of sending client socket
 * @param buffer_capacity Capacity of the message payload to accept
 * @return Received string line
 */
char *read_from_client(int client_fd, size_t buffer_capacity) 
{
    char *buffer = malloc(buffer_capacity);
    for(int i = 0; i < buffer_capacity; i++) 
        buffer[i] = '\0';
    int r = read(client_fd, buffer, buffer_capacity);
    if (r < 0)
        error("Error reading from socket.");
    return buffer;
}

/**
 * Task 1 - 6(a).
 * Processes a 'BYE' command - closes sending client's connection to server socket.
 * @param client_fd Client file descriptor of sending client socket
 * @return 0 - To break message reception loop
 */
int s_bye(int client_fd) 
{
    shutdown(client_fd, SHUT_RDWR);
    if (close(client_fd) < 0) { 
        error("Error closing client socket.");
    }
    printf("*Waves Bye*\n");
    return 0;
}

/**
 * Task 1 - 6(b).
 * Processes a 'GET' command - reads the contents of a file local to the server-side 
 * - and sends it back to sending (client) socket.
 * @param client_fd Client file descriptor of sending client socket
 * @param file_name Name of the file read
 * @return void
 */
void s_get(int client_fd, char *file_name) 
{
    FILE *file = fopen(file_name, "r");
    char file_content[FILE_NAME_BUFFER_CAPACITY];
    if (file == NULL) {
        printf("There was no file name found under GET request for: %s\n", file_name);
        send_to_client(client_fd, "SERVER 404 Not Found\n");
        fclose(file);
        return;
    } else {
        char c;
        int i = 0;
        memset(file_content, 0, FILE_NAME_BUFFER_CAPACITY);
        while((c = fgetc(file)) != EOF) 
            file_content[i++] = c; 
    }
    fclose(file);
    send_to_client(client_fd, "SERVER 200 OK\n\n");
    send_to_client(client_fd, file_content);
    send_to_client(client_fd, "\n\n");
    return;
}

/**
 * Task 1 - 6(c).
 * Continiously reads messages from the sending client socket until two new lines are created.
 * @param client_fd Client file descriptor of sending client socket
 * @return Entire file content for 'PUT' command
 */
char *load_file_content(int client_fd) 
{
    int count = 0;
    char *file_content = malloc(1);
    if (!file_content) {
        send_to_client(client_fd, "SERVER 501 Put Error\n");
        free(file_content);
        return NULL;
    }
    file_content[0] = '\0';
    while(count < 1) {
        char *buffer = read_from_client(client_fd, FILE_CONTENT_BUFFER_CAPACITY);
        if (strcmp(buffer, "\n") == 0) 
            count++;
        size_t file_len = strlen(file_content);
        size_t buff_len = strlen(buffer);

        char *temp = realloc(file_content, file_len + buff_len + 1);
        if (!temp) { 
            free(file_content);
            send_to_client(client_fd, "SERVER 501 Put Error\n");
            break;
        }
        file_content = temp;
        strcat(file_content, buffer);
    }
    return file_content;
}

/**
 * Task 1 - 6(c).
 * Processes a 'PUT' command - writes the contents from the sending client socket to a
 * - new (or overridden) file.
 * @param client_fd Client file descriptor of sending client socket
 * @param file_name Name of the file to create (or override)
 * @return void
 */
void s_put(int client_fd, char *file_name) 
{   
    char *file_content = load_file_content(client_fd);
    FILE *file = fopen(file_name, "w");

    if (file == NULL) {
        printf("There was an error writing to file name found under PUT request for: %s\n", file_name);
        send_to_client(client_fd, "SERVER 501 Put Error\n");
        free(file_content);
        fclose(file);
        return;
    } 
    printf("%s", file_content);
    fprintf(file, "%s", file_content);
    send_to_client(client_fd, "SERVER 200 OK\n\n");
    send_to_client(client_fd, file_content);
    send_to_client(client_fd, "\n\n");
    free(file_content);
    fclose(file);
}

/**
 * Task 1 - 6.
 * Read incoming messages from client socket in the context of a command 
 * - and process it appropriately.
 * @param client_fd Client file descriptor of sending client socket
 * @return 0 or 1 - Continue or break the message reception loop for another iteration.
 */
int read_command(int client_fd) {
    char * buffer = read_from_client(client_fd, COMMAND_BUFFER_CAPACITY);

    char keyword[4];
    strncpy(keyword, buffer, 3);
    keyword[3] = '\0';

    if(strcasecmp(keyword, "BYE") == 0) {
        free(buffer);
        return s_bye(client_fd);
    }

    char file_name[COMMAND_BUFFER_CAPACITY];

    if (strlen(buffer+4) > 0) 
        strncpy(file_name, buffer+4, strlen(buffer+4)-1);
    else {
        send_to_client(client_fd, "SERVER 502 Command Error\n");
        free(buffer);
        return 1;
    }

    if(strcasecmp(keyword, "GET") == 0) {
        s_get(client_fd, file_name);
        free(buffer);
        return 1;
    }

    if(strcasecmp(keyword, "PUT") == 0) {
        s_put(client_fd, file_name);
        free(buffer);
        return 1;
    }
    
    free(buffer);
    send_to_client(client_fd, "SERVER 502 Command Error\n");
    return 1;
}

/**
 * Main.
 * Task 1 - 1.
 * Main function - validates port number from command-line arguments and then invokes all the 
 * appropriate function calls (containing relevant system calls).
 * @param argc Number of command-line arguments
 * @param argv String content of command-line arguments
 * @return 0 or 1 - Exit the process in the context of return code
 */
int main(int argc, char *argv[])
{   
    unsigned short port;

    if(argc < 2 || (port = atoi(argv[1])) < 1024) 
        return -1;

    struct sockaddr_in s_in;
        
    int s_fd = open_socket();
    create_socket(&s_in, port);
    do_bind(s_fd, s_in);
    while(1) {
        int clientfd = listen_and_accept(s_fd);
        pid_t pid = fork();
        if (pid <= 0) {
            send_to_client(clientfd, "Hello\n");
            while(read_command(clientfd)) {}
            exit(0);
        }

    }
    return 0;
}



