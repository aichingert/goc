#include "wcl.h"

void request_site(char const *site) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *servinfo = NULL;

    int res = getaddrinfo(site, "http", &hints, &servinfo);
    if (res != 0) {
        printf("gai error: %s\n", gai_strerror(res));
        exit(1);
    }

    int sock = socket(
        servinfo->ai_family, 
        servinfo->ai_socktype,
        servinfo->ai_protocol
    );
    res = connect(sock, servinfo->ai_addr, servinfo->ai_addrlen);
    if (res < 0) {
        printf("CONNECT FALED\n");
        exit(1);
    }

    char *msg = "GET / HTTP/1.0\r\nHost: stake.com/sports/home\r\nConnection: close\r\n\r\n";
    res = write(sock, msg, strlen(msg));

    if (res < 0) {
        printf("SEND FALED\n");
        exit(1);
    }

    char buf[4096] = {0};
    printf("here\n");
    res = read(sock, buf, sizeof buf);
    printf("there\n");
    if (res == 0) {
        printf("READ FAILED\n");
        exit(1);
    }

    printf("%s\n", buf);

    close(sock);
    freeaddrinfo(servinfo);
}
