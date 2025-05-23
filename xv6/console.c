// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void consputc(int);

static int panicked = 0;

int back_counter = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;


static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
#define NEW_LINE '\n'
#define DOLLAR_SIGN '$'
#define SPACE ' '
#define CONSOLE_COLS 80
#define CONSOLE_ROWS 25
#define ARROW_UP 0xE2
#define ARROW_DOWN 0xE3
#define BEEP '\a'

static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    for(int i = pos - 1; i < pos + back_counter; i++){
      crt[i] = crt[i + 1];
    }
    if(pos > 0) --pos;
  } else {
    for(int i = pos + back_counter; i > pos; i--){ 
      crt[i] = crt[i - 1];
    }
    crt[pos++] = (c&0xff) | 0x0700;  // black on white
  }

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos + back_counter] = ' ' | 0x0700;
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;

#define CMD_STACK_SIZE 10
struct {
  char cmd_buf[CMD_STACK_SIZE][INPUT_BUF];
  uint top;
  uint last_arrow_used;
  uint size;
} cmd_stack;

#define C(x)  ((x)-'@')  // Control-x

static void
put_on_console(const char* s){
  for(int i = 0; i < INPUT_BUF && (s[i]); ++i){
    input.buf[input.e++ % INPUT_BUF] = s[i];
    consputc(s[i]);
  }
}

static void
backspace()
{
  input.e--;
  consputc(BACKSPACE);
}

static void
kill_line(){
  while(input.e != input.w &&
        input.buf[(input.e-1) % INPUT_BUF] != '\n'){
    backspace();
  }
}

static int
get_cursor_pos()
{
  int pos;
  outb(CRTPORT, 14);                  
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);  

  return pos;
}

static void
reset_cursor_pos(int pos)
{
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
}

static int 
is_cursor_beggening_of_line(int pos)  
{
  return (
    input.e = input.w ||
    input.buf[(input.e-1) % INPUT_BUF] == NEW_LINE ||
    crt[pos - 2] == (DOLLAR_SIGN | 0x0700)
  );
}

static void
move_cursor_back()
{ 
  int pos = get_cursor_pos();

  if (is_cursor_beggening_of_line(pos))
  {
    return;
  }
  
  pos--;
  back_counter++;
  reset_cursor_pos(pos);
}

static void
move_cursor_forward()
{
  if (back_counter == 0)
  {
    return;
  }
  
  int pos = get_cursor_pos();
  pos++;
  back_counter--;
  reset_cursor_pos(pos);
}

static void
clear_screen()
{
    for (int row = 0; row < CONSOLE_ROWS; row++) {
        for (int col = 0; col < CONSOLE_COLS; col++) {
            consputc(BACKSPACE);
        }
    }

    consputc(DOLLAR_SIGN);
    consputc(SPACE);
}

static void
push_cmd_stack()
{
  if (input.e - input.w == 1) return;
  
  cmd_stack.top = (cmd_stack.top + 1) % CMD_STACK_SIZE;
  memset(cmd_stack.cmd_buf[cmd_stack.top], 0, INPUT_BUF);
  memmove(cmd_stack.cmd_buf[cmd_stack.top], input.buf + input.w, input.e - input.w - 1);
  cmd_stack.size++;
}

static void
pop_cmd_stack()
{
  if (cmd_stack.size == 0 || cmd_stack.top == 0) return;

  kill_line();
  put_on_console(cmd_stack.cmd_buf[cmd_stack.top]);
  cmd_stack.last_arrow_used = cmd_stack.top;
  cmd_stack.top = (cmd_stack.top - 1 + CMD_STACK_SIZE) % CMD_STACK_SIZE;
}

static void 
reverse_pop_cmd_stack()
{
  if (cmd_stack.size == 0) return;
  
  kill_line();
  put_on_console(cmd_stack.cmd_buf[cmd_stack.last_arrow_used]);
  cmd_stack.top = cmd_stack.last_arrow_used;
  cmd_stack.last_arrow_used = (cmd_stack.last_arrow_used + 1) % CMD_STACK_SIZE;
}

void
consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'):  // Kill line.
      kill_line();
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        backspace();
      }
      break;
    case C('B') : 
      move_cursor_back();
      break;
    case C('F'):
      move_cursor_forward();
      break;  
    case C('L'):
      clear_screen();
      break;  
    case ARROW_UP:
      pop_cmd_stack();
      break;
    case ARROW_DOWN:
      reverse_pop_cmd_stack();
      break;  
    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
          push_cmd_stack();
          back_counter = 0;
          input.w = input.e;
          wakeup(&input.r);
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(myproc()->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  ioapicenable(IRQ_KBD, 0);
}