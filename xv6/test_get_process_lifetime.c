#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

int main(int argc, char *argv[])
{
    int fd = fork();
    
    if (fd < 0) {
        printf(ERROR_FD, "failed to fork!\n");
        exit();
    } else if (fd == 0) {
        printf(WRITE_FD, "child process, PID: %d\nlifetime: %d\n",
            getpid(), get_process_lifetime(fd));
        exit();
    } else {
        printf(WRITE_FD, "parent process, PID: %d\nlifetime: %d\n",
            getpid(), get_process_lifetime(fd));

        wait();
        
        exit();    
    }
    
    exit();
}