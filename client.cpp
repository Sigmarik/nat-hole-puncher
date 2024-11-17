#include <arpa/inet.h>
#include <assert.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>

#include "addr_pack.h"

int main(int argc, char **argv) {
    if (argc < 5) {
        std::cerr << "Usage: " << basename(argv[0]) << " ip port passes"
                  << std::endl;
        return EXIT_FAILURE;
    }

    const char *ip_string = argv[1];
    const char *port_string = argv[2];
    const char *passes_string = argv[3];

    addrinfo hints;
    hints.ai_family = AF_INET;
    struct addrinfo *res;
    int ret = getaddrinfo(ip_string, NULL, &hints, &res);
    if (ret != 0) {
        std::cerr << "Unable to resolve host '" << ip_string << "'."
                  << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in sock_addr;
    socklen_t addr_len;
    memcpy(&sock_addr, res->ai_addr, res->ai_addrlen);
    addr_len = res->ai_addrlen;

    freeaddrinfo(res);

    int port = atoi(port_string);
    sock_addr.sin_port = htons((uint16_t)port);

    int passes = atoi(passes_string);
    uint32_t real_passes = 0;
    if (passes <= 0) {
        std::cerr << "Passes (" << passes << ") must be positive." << std::endl;
        return EXIT_FAILURE;
    }

    real_passes = 2 * passes;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    AddressPack last_addr;

    sendto(sock, &last_addr, sizeof(last_addr), 0, (const sockaddr *)&sock_addr,
           addr_len);

    recvfrom(sock, &last_addr, sizeof(last_addr), MSG_TRUNC, NULL, NULL);

    struct sockaddr_in remote_addr;
    int remote_set;
    uint32_t iter;
    if (last_addr.ip == 0 && last_addr.port == 0) {
        remote_set = 0;
        iter = 0;
    } else {
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = last_addr.ip;
        remote_addr.sin_port = last_addr.port;
        remote_set = 1;
        iter = 1;

        sock = socket(AF_INET, SOCK_DGRAM, 0);

        real_passes++;
    }

    for (; iter < real_passes; iter++) {
        if (!(iter & 1)) {
            uint32_t pass_buf;
            pass_buf = 0;

            socklen_t len = sizeof(remote_addr);

            recvfrom(sock, &pass_buf, sizeof(pass_buf), MSG_TRUNC,
                     (sockaddr *)(remote_set ? NULL : &remote_addr),
                     remote_set ? NULL : &len);

            remote_set = 1;
            printf("Received '%u'\n", ntohl(pass_buf));
        } else {
            assert(remote_set);
            uint32_t pass_buf = htonl((uint32_t)iter / 2);

            sendto(sock, &pass_buf, sizeof(pass_buf), 0,
                   (const sockaddr *)&remote_addr, sizeof(remote_addr));
        }
    }

    return EXIT_SUCCESS;
}
