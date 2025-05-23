#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(find_digital_root)
SYSCALL(copy_file)
SYSCALL(get_uncle_count)
SYSCALL(get_process_lifetime)
SYSCALL(change_process_queue)
SYSCALL(set_bjf_process)
SYSCALL(set_bjf_system)
SYSCALL(print_info)
SYSCALL(initpriority)
SYSCALL(testpriority)
SYSCALL(getsyscallnum)
SYSCALL(resetsyscallnum)
