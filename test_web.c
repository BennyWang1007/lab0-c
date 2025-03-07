#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 9999

/* length of unique message (TODO below) should shorter than this */
#define MAX_MSG_LEN 1024
static const char *msg_dum = "GET /new HTTP/1.1\n\n";

int main(void)
{
    int sock_fd;
    char dummy[MAX_MSG_LEN];
    // struct timeval start, end;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in info = {
        .sin_family = PF_INET,
        .sin_addr.s_addr = inet_addr(TARGET_HOST),
        .sin_port = htons(TARGET_PORT),
    };

    if (connect(sock_fd, (struct sockaddr *) &info, sizeof(info)) == -1) {
        perror("connect");
        exit(-1);
    }
    send(sock_fd, msg_dum, strlen(msg_dum), 0);
    recv(sock_fd, dummy, MAX_MSG_LEN, 0);

    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);

    printf("%s\n", dummy);

    int count = 0, last_pos = 0;
    for (int i = 0; i < strlen(dummy); i++) {
        if (dummy[i] == '\n') {
            count++;
        }
        if (count == 4) {
            last_pos = i + 1;
            break;
        }
    }

    char const *answer = "l = []\n";
    printf("%d %d\n", (int) strlen(answer), (int) strlen(dummy + last_pos));

    if (strncmp(answer, dummy + last_pos, strlen(answer)) == 0) {
        printf("Match\n");
    } else {
        printf("No match\n");
    }

    return 0;
}