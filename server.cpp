#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <iostream>

#include "addr_pack.h"

int main(int argc, char **argv) {
    unsigned port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    std::cout << "Opening a socket..." << std::endl;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (const sockaddr *)&server_addr, sizeof(server_addr));

    std::cout << "Listening to incoming rendezvous. Port: " << port
              << std::endl;

    AddressPack last_addr;

    for (unsigned long long iteration = 0;; ++iteration) {
        sockaddr_in next_data;
        socklen_t next_len = sizeof(next_data);

        recvfrom(sock, NULL, 0, MSG_TRUNC, (sockaddr *)&next_data, &next_len);

        sendto(sock, (void *)&last_addr, sizeof(last_addr), 0,
               (sockaddr *)&next_data, sizeof(next_data));

        if (iteration & 1) {
            last_addr = AddressPack();
        } else {
            last_addr.ip = next_data.sin_addr.s_addr;
            last_addr.port = next_data.sin_port;
        }
    }

    return EXIT_SUCCESS;
}
