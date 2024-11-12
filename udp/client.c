#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 3000
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 4096
#define NUM_PACKETS 10  // Number of packets to send

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serverAddr;
    char *htmlContent = "<!DOCTYPE html>\n"
                        "<html lang=\"en\">\n"
                        "<head>\n"
                        "    <meta charset=\"UTF-8\">\n"
                        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                        "    <title>UDP Data from Client</title>\n"
                        "</head>\n"
                        "<body>\n"
                        "    <h1>UDP HTML Content</h1>\n"
                        "</body>\n"
                        "</html>";

    char request[BUFFER_SIZE];
    int bytesSent, bytesReceived;
    char response[BUFFER_SIZE];
    int serverAddrLen = sizeof(serverAddr);
    int totalPacketsSent = 0, totalPacketsReceived = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Record the start time of the entire transaction
    clock_t totalStart = clock();

    // Send multiple packets
    for (int i = 0; i < NUM_PACKETS; i++) {
        snprintf(request, sizeof(request),
                 "POST / HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n"
                 "%s", SERVER_ADDR, SERVER_PORT, (int)strlen(htmlContent), htmlContent);

        clock_t start = clock();
        bytesSent = sendto(sock, request, strlen(request), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        if (bytesSent < 0) {
            printf("Failed to send request.\n");
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        bytesReceived = recvfrom(sock, response, sizeof(response) - 1, 0, NULL, NULL);
        if (bytesReceived > 0) {
            response[bytesReceived] = '\0';
            totalPacketsReceived++;
        }
        clock_t end = clock();

        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Round-trip time for packet %d: %.6f seconds\n", i + 1, time_taken);

        totalPacketsSent++;
    }

    // Record the total end time
    clock_t totalEnd = clock();
    double totalTime = ((double)(totalEnd - totalStart)) / CLOCKS_PER_SEC;
    double packetLoss = (1 - ((double)totalPacketsReceived / totalPacketsSent)) * 100;

    // printf("Packet loss: %.2f%%\n", packetLoss);
    printf("Total time taken for sending and receiving %d packets: %.6f seconds\n", NUM_PACKETS, totalTime);

    closesocket(sock);
    WSACleanup();
    return 0;
}
