#include "types.h"
#include "user.h"
#include "param.h"

int main(int argc, char* argv[])
{
    printf(WRITE_FD, "Process ID: %d\n", getpid());
    exit();
}