#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        printf(ERROR_FD, "usage: cp <src> <dist>\n");
        exit();
    }

    char* src = argv[1];
    char* dist = argv[2];
    int result = copy_file(src, dist);
    
    if (result == -1) {
        printf(ERROR_FD, "failed copy %s to %s!\n", src, dist);
        exit();
    }
    
    printf(WRITE_FD, "%s copied to %s successfully\n", src, dist);

    exit();
}