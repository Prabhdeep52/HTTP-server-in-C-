#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096

// Global variable to store HTML content received from the client
char storedHtmlContent[BUFFER_SIZE] = "<html><body><h2>No content received yet</h2></body></html>";

DWORD WINAPI handleClient(LPVOID clientSocket) {
    SOCKET newsockfd = *(SOCKET *)clientSocket;
    char buffer[BUFFER_SIZE];

    // Read data from the socket
    int valread = recv(newsockfd, buffer, BUFFER_SIZE - 1, 0);
    if (valread == SOCKET_ERROR) {
        printf("Error in receiving data: %d\n", WSAGetLastError());
        closesocket(newsockfd);
        return 1;
    }
    buffer[valread] = '\0';

    printf("Received request:\n%s\n", buffer);

    // Check if the request is for the /display page (GET request)
    if (strncmp(buffer, "GET /display", 12) == 0) {
        // Serve the stored HTML content
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Connection: close\r\n\r\n"
                 "%s", storedHtmlContent);
        send(newsockfd, response, strlen(response), 0);
    }
    // Check if the request is a POST request
    else if (strncmp(buffer, "POST", 4) == 0) {
        // Find the start of the POST body
        char *body = strstr(buffer, "\r\n\r\n");
        if (body != NULL) {
            body += 4; // Skip past the headers to the body

            // Store received HTML content in global variable
            snprintf(storedHtmlContent, sizeof(storedHtmlContent), "%s", body);

            // Send acknowledgment response
            const char *response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n\r\n"
                                   "<html><body><h2>HTML content received successfully! Go to <a href=\"/display\">/display</a> to view it.</h2></body></html>";
            send(newsockfd, response, strlen(response), 0);
        }
    } else {
        // Handle unsupported request methods
        const char *response = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/html\r\n"
                               "Connection: close\r\n\r\n"
                               "<html><body><h1>404 Not Found</h1></body></html>";
        send(newsockfd, response, strlen(response), 0);
    }

    closesocket(newsockfd);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET sockfd, newsockfd;
    struct sockaddr_in serverAddr;
    int addrLen = sizeof(serverAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr, addrLen) != 0) {
        printf("Socket binding failed\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    if (listen(sockfd, SOMAXCONN) != 0) {
        printf("Socket listen failed\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&serverAddr, &addrLen);
        if (newsockfd == INVALID_SOCKET) {
            printf("Connection accept failed: %d\n", WSAGetLastError());
            continue;
        }
        printf("Connection accepted\n");

        // Create a thread to handle each client connection
        HANDLE thread = CreateThread(NULL, 0, handleClient, &newsockfd, 0, NULL);
        if (thread == NULL) {
            printf("Thread creation failed\n");
            closesocket(newsockfd);
        } else {
            CloseHandle(thread); // Close thread handle to prevent memory leaks
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}


