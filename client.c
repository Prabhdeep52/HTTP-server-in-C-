#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>  // For measuring time

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 4096

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serverAddr;
 char *htmlContent = "<!DOCTYPE html>\n"
                    "<html lang=\"en\">\n"
                    "<head>\n"
                    "    <meta charset=\"UTF-8\">\n"
                    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                    "    <title>Centered Content</title>\n"
                    "    <style>\n"
                    "        body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }\n"
                    "        h1 { text-align: center; }\n"
                    "    </style>\n"
                    "</head>\n"
                    "<body>\n"
                    "    <h1>TCP Data sent by client</h1>\n"
                    "</body>\n"
                    "</html>";


    char request[BUFFER_SIZE];
    int bytesSent, bytesReceived;
    char response[BUFFER_SIZE];
    clock_t start, end;  // Variables to store start and end time

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    // Setup server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Start measuring time
    start = clock();

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Connection failed.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Create HTTP POST request with HTML content
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n\r\n"
             "%s",
             SERVER_ADDR, SERVER_PORT, (int)strlen(htmlContent), htmlContent);

    // Send request to server
    bytesSent = send(sock, request, strlen(request), 0);
    if (bytesSent < 0) {
        printf("Failed to send request.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Request sent successfully.\n");

    // Receive response from server
    bytesReceived = recv(sock, response, sizeof(response) - 1, 0);
    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';  // Null-terminate the response
        printf("Server response:\n%s\n", response);
    } else {
        printf("Failed to receive response.\n");
    }

    // End measuring time
    end = clock();

    // Calculate the time taken in seconds
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time taken for the request and response: %f seconds\n", time_taken);

    // Cleanup
    closesocket(sock);
    WSACleanup();
    return 0;
}


