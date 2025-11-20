#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "killresp.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"message\"\n", argv[0]);
        return 1;
    }

    // make a writable copy of argv[1]
    char msg[256];
    strncpy(msg, argv[1], sizeof(msg)-1);
    msg[sizeof(msg)-1] = '\0';

    for (char *p = msg; *p; p++) { // handle "\n" sent from shell (shell dosent handle it without certain syntaxing)
        if (*p == '\\' && *(p+1) == 'n') {
            *p = '\n';
            memmove(p+1, p+2, strlen(p+2)+1);
        }
    }

    // actual program below

    killresp(SIGTERM, msg); // returns immediately. your program's logic would go below

    printf("PID %d, Waiting for SIGTERM\n", getpid());
    while (1) pause(); // keep running

    return 0;
}
