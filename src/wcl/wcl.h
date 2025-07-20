#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include<stdlib.h>
#include<stddef.h>

#define LEN(x) sizeof(x) / sizeof(x[0])

void request_site(char const *site);
