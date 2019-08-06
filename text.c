#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

struct termios orig_termios;

void disableRaw()
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void readRaw()
{
  struct termios raw;
  tcgetattr(STDIN_FILENO, &orig_termios);  //gets input from stdin and stores them in global
  atexit(disableRaw); //disable raw mode upon exit
  raw = orig_termios; //we make a copy of orig_termios before making changes
  raw.c_lflag &= ~(IXON); //sets the IXON flag so that CTRL-S and CTRL-Q macros are disabled, instead interpreted as a 19
                          //byte and 17 byte input, respectively
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); /* We will set ECHO, ICANON, ISIG, and IEXTEN flags so we can:
                          / 1.) render a UI in raw mode (ECHO)
                          / 2.) render a UI in noncanonical mode (accept input continuously) (ICANON)
                          / 3.) disable CTRL-C and CTRL-Z functions, accepting them as a 3 byte and 26 byte input, respectively (ISIG)
                          / 4.) 
			  / We use bitwise NOT on ECHO, ICANON flags, and ISIG flags (OR'd together so we can set each at once)
                          / and bitwise AND their value with the flags field to fource every fourth
                          / bit to become 0, allowing every other bit to retain its current
                          / value. NOTE: We set the IXON flag before the other flags because it is an input flag and thus belongs
                          / to the input flag field, whereas the other flags belong to the c_lflag field. The naming convention is 
                          / particularly cofusing because ICANON, ISIG, IEXTEN all have an 'I' prefix but do not belong to the input flag field.
                          */
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  /* Sets the parameters associated with the terminal from termios struct (raw)
  / (i.e. sets current terminal control settings for stdin and stores in raw).
  / TCSAFLUSH arg. indicates that no change will be made to stdin until all currently
    written data has been transmitted (received but unread data is discarded at this
    point)
  */
} 

int main()
{
  readRaw();

  char buff;
   
  while (read(STDIN_FILENO, &buff, 1) == 1 && buff != 'q') //indefinitely take input from user until 'q' is pressed
  {
    if (iscntrl(buff)) //test whether character is control character (nonprintable)
    {
      printf("%d\n", buff); //if so, format byte as decimal number (ASCII code)
    }
    else
    {
      printf("%d ('%c')\n", buff, buff);
    }
  } 
  return 0;
}

