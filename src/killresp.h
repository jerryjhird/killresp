#ifndef KILLRESP_H
#define KILLRESP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/uio.h>

static char signal_msg[256];

static void sighandle(int sig, siginfo_t *si, void *ucontext) {
    (void)sig; (void)ucontext;
    if (!si) return;

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%d/fd/1", si->si_pid);

    int fd = open(path, O_WRONLY);
    if (fd != -1) {
        struct iovec iov = { .iov_base = signal_msg, .iov_len = strlen(signal_msg) };
        writev(fd, &iov, 1);
        close(fd);
    }
}

void killresp(int signo, const char *msg) {
    if (!msg) {
        fprintf(stderr, "killresp: message is NULL\n");
        exit(1);
    }

    strncpy(signal_msg, msg, sizeof(signal_msg)-1);
    signal_msg[sizeof(signal_msg)-1] = '\0';

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sighandle;
    if (sigaction(signo, &sa, NULL) != 0) {
        perror("sigaction");
        exit(1);
    }
}

#endif