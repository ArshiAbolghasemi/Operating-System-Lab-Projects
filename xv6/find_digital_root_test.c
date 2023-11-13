#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

int syscall_find_digital_root(int num)
{
    int prev_ebx;

    // Save ebx in prev_ebx to restore later.
    // Move num to ebx.
    asm volatile(
        "movl %%ebx, %0\n\t"
        "movl %1, %%ebx"
        : "=r"(prev_ebx)
        : "r"(num)
    );

    int result = find_digital_root();

    // Restore ebx.
    asm volatile(
        "movl %0, %%ebx"
        :: "r"(prev_ebx)
    );

    return result;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf(ERROR_FD, "usage: find_digital_root_test <number>\n");
        exit();
    }

    int number = atoi(argv[1]);
    int digital_root = syscall_find_digital_root(number);
    
    if (digital_root == -1) {
        printf(ERROR_FD, "number should be positive!\nnumber: %d\n", number);
        exit();
    }
    
    printf(WRITE_FD, "%d\n", digital_root);

    exit();
}