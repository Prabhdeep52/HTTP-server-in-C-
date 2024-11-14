#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 3000
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 4096
#define NUM_PACKETS 10

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
    double totalRoundTripTime = 0.0;

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

    clock_t totalStart = clock();

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
        clock_t end = clock();

        double roundTripTime = ((double)(end - start)) / CLOCKS_PER_SEC;

        if (bytesReceived > 0) {
            response[bytesReceived] = '\0';
            totalPacketsReceived++;
            printf("UDP Round-trip time for packet %d: %.6f seconds\n", i + 1, roundTripTime);
            totalRoundTripTime += roundTripTime;
        } else {
            printf("Packet %d lost.\n", i + 1);
        }

        totalPacketsSent++;
    }


clock_t totalEnd = clock();
double totalTime = ((double)(totalEnd - totalStart)) / CLOCKS_PER_SEC;
double packetLoss = (1 - ((double)totalPacketsReceived / totalPacketsSent)) * 100;

printf("\n--- UDP Statistics ---\n");
printf("Total packets sent: %d\n", totalPacketsSent);
printf("Total packets received: %d\n", totalPacketsReceived);
printf("Packet loss: %.2f%%\n", packetLoss);
printf("Average round-trip time: %.6f seconds (%.2f ms)\n", totalRoundTripTime / totalPacketsReceived, 
       (totalRoundTripTime / totalPacketsReceived) * 1000);
printf("Total time taken for all packets: %.6f seconds (%.2f ms)\n", totalTime, totalTime * 1000);


    closesocket(sock);
    WSACleanup();
    return 0;
}
