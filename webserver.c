#include<stdio.h>
#include <errno.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8080
#define BUFFER_SIZE 1024
// socket creates an endpoint for communication . 
int main(){
       // Initialize Winsock . winsock is a library that contains functions for networking . 
    WSADATA wsa;
    // here we initialize winsock to use the functions in the library .                     
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // 2.2 is the version of winsock we want to use , &wsa is pointer to a structure (WSADATA) where the startup information will be stored.
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create a socket

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //This line creates a TCP socket (SOCK_STREAM) using the IPv4 address family (AF_INET). The 0 parameter indicates the default protocol for the given address family and socket type.
    if (sockfd == INVALID_SOCKET) {
        printf("Error creating socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created successfully\n");

    // now we have to bind the socket to an address and port of our local machine. 


    // Create the address to bind the socket to
    struct sockaddr_in host_address; // data structure that contains the address we want to bind to
    int host_addr_length = sizeof(host_address);

    host_address.sin_family = AF_INET; // AF_INET is the address family for IPv4
    host_address.sin_port = htons(PORT);//htons() is used to convert the port number PORT from host byte order to network byte order (which is necessary for networking functions).
    host_address.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY is a constant that represents any available IP address on the host machine. 

    // Bind the socket to our specified IP and port
    if (bind(sockfd, (struct sockaddr *)&host_address, host_addr_length) != 0) { // returns 0 if successful 
        perror("webserver (bind)");
        return 1;
    }
    printf("socket successfully bound to address\n");

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) {  // somaxconn is the maximum number of connections that can be waiting while the process is handling a particular connection. it is a int defined in winsock. 
        perror("webserver (listen)");
        return 1;
    }
    printf("server listening for connections\n");


    // Accept an incoming connection request
    for (;;) {
        // listen to the incoming request and create a new socket for every client .
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_address, (socklen_t *)&host_addr_length);
      
        if (newsockfd < 0) {
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        // Read from the socket
        char buffer[BUFFER_SIZE];
        
        int valread = recv(newsockfd, buffer, BUFFER_SIZE, 0);

        // error handling 
        if (valread == SOCKET_ERROR) {
            printf("Error in receiving data: %d\n", WSAGetLastError());
            closesocket(newsockfd);
            continue;
        }

        if (valread == 0) {
            printf("Client disconnected\n");
            closesocket(newsockfd);
            continue;
        }
        if (valread < 0) {
            perror("webserver (read)");
            continue;
        }

        // Print the data received
        printf("Received data: %s\n", buffer);

        // Send a response
char response[] = "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/html\r\n"
                  "Connection: close\r\n\r\n"
                  "<!DOCTYPE html>\r\n"
                  "<html>\r\n"
                  "<head>\r\n"
                  "<title>Server in C </title>\r\n"
                  "</head>\r\n"
                  "<body>\r\n"
                  "<h1>Hello, World!</h1>\r\n"
                 "</body>\r\n"
                  "</html>\r\n";


        int write_data = send(newsockfd, response, strlen(response), 0);

        if (write_data == SOCKET_ERROR) {
            printf("Error in sending data: %d\n", WSAGetLastError());
            // Handle error
        } 
        else if (write_data < 0) {
            perror("webserver (write)");
            continue;
        }
        else {
            printf("Sent %d bytes of data\n", write_data);
            // Data sent successfully
        }

       closesocket(newsockfd);

    }

    // Clean up Winsock
    closesocket(sockfd);
    WSACleanup();
}