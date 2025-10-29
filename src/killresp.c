#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

int sig_pipe[2] = {-1, -1};

void sigterm_h(int sig, siginfo_t *si, void *ucontext) {
    (void)sig;
    (void)ucontext;

    if (!si) return;

    int32_t sender = (int32_t)si->si_pid;
    if (sig_pipe[1] != -1) {
        ssize_t r = write(sig_pipe[1], &sender, sizeof(sender));
        (void)r;
    }
}

int send_to_pid_stdout(pid_t pid, const char *msg) {
    char fdpath[PATH_MAX];
    char target[PATH_MAX + 1];
    ssize_t r;
    struct stat st;

    if (!msg) { errno = EINVAL; return -1; }

    snprintf(fdpath, sizeof(fdpath), "/proc/%d/fd/1", (int)pid);
    r = readlink(fdpath, target, PATH_MAX);
    if (r == -1) return -1;
    if ((size_t)r >= sizeof(target)) return -1;
    target[r] = '\0';

    if (target[0] != '/') {
        errno = ENOTSUP;
        return -1;
    }

    if (stat(target, &st) == -1) return -1;

    int flags = O_WRONLY;
    if (S_ISREG(st.st_mode)) flags |= O_APPEND;

    int fd = open(target, flags | O_CLOEXEC);
    if (fd == -1) return -1;

    size_t len = strlen(msg);
    const char *p = msg;
    while (len > 0) {
        ssize_t w = write(fd, p, len);
        if (w == -1) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        p += w;
        len -= (size_t)w;
    }

    close(fd);
    return 0;
}

static void *worker_thread(void *arg) {
    (void)arg;
    for (;;) {
        int32_t sender32;
        ssize_t r = read(sig_pipe[0], &sender32, sizeof(sender32));
        if (r == 0) {
            break;
        } else if (r < 0) {
            if (errno == EINTR) continue;
            perror("worker read");
            break;
        } else if (r != sizeof(sender32)) {
            continue;
        }

        pid_t sender = (pid_t)sender32;
        if (sender <= 0) continue;

        const char *message =
            "i saw you sigterm\n";

        if (send_to_pid_stdout(sender, message) == 0) {
            printf("[async] Sent message to pid %d stdout\n", (int)sender);
        } else {
            int e = errno;
            fprintf(stderr, "[async] failed to send to pid %d stdout: %s\n", (int)sender, strerror(e));
        }
    }
    return NULL;
}

int main(void) {
    if (pipe(sig_pipe) == -1) {
        perror("pipe");
        return 1;
    }

    if (fcntl(sig_pipe[0], F_SETFD, FD_CLOEXEC) == -1) perror("fcntl");
    if (fcntl(sig_pipe[1], F_SETFD, FD_CLOEXEC) == -1) perror("fcntl");

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigterm_h;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("sigaction");
        return 1;
    }

    /* start worker thread */
    pthread_t tid;
    if (pthread_create(&tid, NULL, worker_thread, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }

    printf("PID %d\n", getpid());

    for (;;) {
        sleep(2);
    }

    close(sig_pipe[0]);
    close(sig_pipe[1]);
    pthread_join(tid, NULL);
    return 0;
}
