// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "stdarg.h"
#include "param.h"

char *argv[] = { "sh", 0 };

void 
print_group_memebers_name(int group_id, int gropu_member_cnt, ...);

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  print_group_memebers_name(12, 3, "Golbol Rashidi", "Erfan Ahmadi", "Arshia Abolghasemi");

  for(;;){
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}

void 
print_group_memebers_name(int group_id, int gropu_members_cnt, ...)
{
  va_list names;
  va_start(names, gropu_members_cnt);

  printf("Group #%i\n", group_id);

  for (int i = 0; i < gropu_members_cnt; i++)
  {
    printf(WRITE_FD, "%i. %s\n", i + 1, va_arg(names, char*));
  }

  va_end(names);
  return;
}
