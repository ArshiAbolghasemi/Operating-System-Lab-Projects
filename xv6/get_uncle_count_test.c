#include "types.h"
#include "user.h"
#include "param.h"

int main(void) {
    int pid1, pid2, pid3;

    pid1 = fork();

    if (pid1 < 0) {
        printf(ERROR_FD, "fork 1 failed\n");
    } else if (pid1 == 0) {
        printf(WRITE_FD, "child 1 process, PID: %d\n", getpid());
        exit();
    } else {
        printf(WRITE_FD, "parent process, child 1 PID: %d\n", pid1);
        pid2 = fork();

        if (pid2 < 0) {
            printf(ERROR_FD, "fork 2 failed\n");
        } else if (pid2 == 0) {
            printf(WRITE_FD, "child 2 process, PID: %d\n", getpid());
            exit();
        } else {
            printf(WRITE_FD, "parent process, child 2 PID: %d\n", pid2);
            pid3 = fork();

            if (pid3 < 0) {
                printf(ERROR_FD, "fork 3 failed\n");
            } else if (pid3 == 0) {
                printf(ERROR_FD, "child 3 process, PID: %d\n", getpid());

                int gpid = fork();

                if (gpid < 0) {
                    printf(ERROR_FD, "grandchild Fork failed\n");
                } else if (gpid == 0) {
                    printf(
                        WRITE_FD,
                        "grandchild process, PID: %d\nuncle count: %d\n",
                        getpid(), get_uncle_count(getpid()));
                    exit();
                } else {
                    wait();
                    exit();
                }
                exit();
            } else {
                printf(WRITE_FD, "parent process, child 3 PID: %d\n", pid3);

                wait();
                wait();
                wait();

                exit();
            }
        }
    }

    printf(ERROR_FD, "something goes wring!\n");
    exit();
}
