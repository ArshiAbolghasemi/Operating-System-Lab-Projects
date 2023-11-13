#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
 
int 
sys_find_digital_root(void) 
{
    int number = myproc()->tf->ebx;

    if (number < 0) return - 1;
        
    int sum = 0;
    while (number > 0) {
        sum += number % 10;       
        number /= 10;
    }

    return sum;
}
