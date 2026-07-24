#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CHILDREN 5
#define TIMEOUT 5

pid_t child_pids[MAX_CHILDREN];
int child_count = 0;

void handle_alarm(int sig) {
    printf("\n[PARENT] Timeout reached. Killing unresponsive children...\n");
    for (int i = 0; i < child_count; i++) {
        if (child_pids[i] > 0) {
            printf("[PARENT] Sending SIGKILL to PID %d\n", child_pids[i]);
            kill(child_pids[i], SIGKILL);
        }
    }
}

void handle_sigchld(int sig) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status))
            printf("[PARENT] Reaped child PID %d (exit: %d)\n", pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("[PARENT] Reaped child PID %d (killed by signal %d)\n", pid, WTERMSIG(status));
        for (int i = 0; i < child_count; i++) {
            if (child_pids[i] == pid) {
                child_pids[i] = 0;
                break;
            }
        }
    }
}

void do_work(int id, int sleep_time) {
    printf("[CHILD %d] PID %d: Working for %d seconds\n", id, getpid(), sleep_time);
    sleep(sleep_time);
    printf("[CHILD %d] PID %d: Done\n", id, getpid());
    exit(0);
}

int main() {
    printf("[PARENT] PID: %d\n", getpid());
    signal(SIGALRM, handle_alarm);
    signal(SIGCHLD, handle_sigchld);

    for (int i = 0; i < MAX_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {
            if (i % 2 == 0)
                do_work(i + 1, 2);
            else
                do_work(i + 1, 10);
        }
        child_pids[child_count++] = pid;
        printf("[PARENT] Created child PID %d\n", pid);
    }

    printf("\n[PARENT] Setting %d sec alarm for slow children\n", TIMEOUT);
    alarm(TIMEOUT);
    printf("[PARENT] Monitoring children...\n");

    int remaining;
    do {
        remaining = 0;
        for (int i = 0; i < child_count; i++)
            if (child_pids[i] > 0) remaining++;
        if (remaining > 0) sleep(1);
    } while (remaining > 0);

    printf("\n[PARENT] All done. Zombies prevented.\n");
    return 0;
}