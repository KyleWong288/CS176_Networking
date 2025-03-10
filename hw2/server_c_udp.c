/*

Citations:

I copied anything socket related from the socket.h manpage.
This includes usage of socket(), AF_INET, SOCK_DGRAM, sendto(), recvfrom(), bind(), close().
socket.h manpage: https://man7.org/linux/man-pages/man2/socket.2.html

I copied anything address handling related from the arpa/inet.h manpage and netinet/in.h manpage. 
This includes usage of the sockaddr_in struct, INADDR_ANY, htons().
arpa/inet.h manpage: https://man7.org/linux/man-pages/man0/arpa_inet.h.0p.html
netinet/in.h manpage: https://man7.org/linux/man-pages/man0/netinet_in.h.0p.html

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

// returns 1 if input is all digits
int validate_input(char* input) {
    int res = 1;
    int len = strlen(input);
    for (int i=0; i<len; ++i) {
        res = res && isdigit(input[i]);
    }
    res = res && len;
    return res;
}

// returns digit sum of a string input
int digit_sum_str(char* input) {
    int res = 0;
    int len = strlen(input);
    for (int i=0; i<len; ++i) {
        res += (input[i] - '0');
    }
    return res;
}

// returns digit sum of an int input
int digit_sum_int(int input) {
    int res = 0;
    while (input) {
        res += input % 10;
        input /= 10;
    }
    return res;
}

// ARGS: port number
int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    const int PORT = atoi(argv[1]);
    const int MAX_BUFFER_SIZE = 1024;
    char buffer[MAX_BUFFER_SIZE];

    // Configure server socket:
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Configure server address:
    struct sockaddr_in server_address, client_address;
    memset(&server_address, 0, sizeof(server_address));
    socklen_t client_address_length = sizeof(client_address);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the specified port:
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        printf("Error binding socket\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // printf("UDP server opened on port %d...\n", PORT);

    while (1) {
        // Receive initial string from client:
        memset(buffer, 0, sizeof(buffer));
        int received_bytes = recvfrom(server_socket, buffer, MAX_BUFFER_SIZE, 0,
                                         (struct sockaddr *)&client_address, &client_address_length);
        buffer[received_bytes] = '\0';

        // Invalid format, return failure to compute:
        if (!validate_input(buffer)) {
            strcpy(buffer, "From server: Sorry, cannot compute!");
            int response_bytes = strlen(buffer);
            sendto(server_socket, buffer, response_bytes, 0,
               (struct sockaddr *)&client_address, client_address_length);
            continue;
        }

        // Compute first digit sum and sends to client:
        int sum = digit_sum_str(buffer);
        sprintf(buffer, "From server: %d", sum);
        int response_bytes = strlen(buffer);
        sendto(server_socket, buffer, response_bytes, 0,
               (struct sockaddr *)&client_address, client_address_length);

        // Continually compute digit sums and sends to client:
        while (sum > 9) {
            sum = digit_sum_int(sum);
            sprintf(buffer, "From server: %d", sum);
            response_bytes = strlen(buffer);
            sendto(server_socket, buffer, response_bytes, 0,
               (struct sockaddr *)&client_address, client_address_length);
        }
    }

    // Close server socket:
    close(server_socket);

    return 0;
}