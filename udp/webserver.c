#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 3000
#define BUFFER_SIZE 4096

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Bind failed\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("UDP Server is waiting for data on port %d...\n", PORT);

    while (1) {
        int bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&clientAddr, &addrLen);
        if (bytesReceived < 0) {
            printf("Receive failed\n");
            continue;
        }
        buffer[bytesReceived] = '\0';

        printf("Received packet from client: \n%s\n", buffer);

        // Send response back to client
        sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&clientAddr, addrLen);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
