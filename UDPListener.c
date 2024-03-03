#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int server_socket;
struct sockaddr_in server_address;

int main() {
        printf("UDP Server start\n");

    // Create UDP socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    // Bind socket to address and port
    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on port %d...\n", PORT);

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    char buffer[BUFFER_SIZE];

    while(1) {
        // Receive message from client
        // memset(buffer, 0, BUFFER_SIZE);
        ssize_t received_bytes = recvfrom(server_socket, buffer, BUFFER_SIZE, 0,
                                          (struct sockaddr *)&client_address, &client_address_len);
        if (received_bytes == -1) {
            perror("Error receiving message");
            continue;
        }

        // Print received message
        printf("Received message from %s:%d: %s", inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port), buffer);

        // // Send response back to client
        // if (sendto(server_socket, buffer, received_bytes, 0,
        //            (struct sockaddr *)&client_address, client_address_len) == -1) {
        //     perror("Error sending response");
        //     continue;
        // }

        // printf("Response sent.\n");
    }

    close(server_socket);
    return 0;
}
