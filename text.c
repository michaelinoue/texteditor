#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/* Written by Michael Inoue, 2019
/  Relevant Documentation: 
/  Flags:
/  	https://www.gnu.org/software/libc/manual/html_node/Local-Modes.html
/  	https://www.gnu.org/software/libc/manual/html_node/Input-Modes.html
/  	https://www.gnu.org/software/libc/manual/html_node/Output-Modes.html
/  	https://www.gnu.org/software/libc/manual/html_node/Control-Modes.html
/  Termios:
/  	http://man7.org/linux/man-pages/man3/termios.3.html
*/

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
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | IXON | ISTRIP); //sets the IXON flag so that CTRL-S and CTRL-Q macros are disabled, instead interpreted as a 19
                          //byte and 17 byte input, respectively
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); 
                          
                          /* We will turn off the above flags so we can:
                          / 1.) render a UI in raw mode (ECHO)
                          / 2.) render a UI in noncanonical mode (accept input continuously) (ICANON)
                          / 3.) disable CTRL-C and CTRL-Z functions, accepting them as a 3 byte and 26 byte input, respectively (ISIG)
                          / 4.) disable CTRL-V functions, accepting them a 3 byte input (IEXTEN)
                          / 5.) disable CTRL-S and CTRL-Q macros, accepting them as a 19 byte and 17 byte input, respectively (IXON)
                          / 6.) disable CTRL-M and ENTER functions, accepting them as a 13 byte input (carriage return)
                          / 7.) disable output processing features (e.g. \n) (OPOST)
                          / 8.) disallow a break condition to send a SIGINT signal to the program (BRKINT)
                          / 9.) disable parity checking (INPCK)
                          / 10.) disallow the 8th input byte from being stripped (ISTRIP)
                          / (not a flag, but a bitmask; however, still relevant enough to mention):
                          / 11.) set character size to 8 bits per byte (CS8)
			  / We use bitwise NOT on these flags (OR'd together by flag field so we can set each at once)
                          / and bitwise AND their value with the flags field to fource every fourth
                          / bit to become 0, allowing every other bit to retain its current
                          / value. 
                          / NOTE: the flag fields are significant. The flags OR'd together belong to the same field (i.e. ECHO, ICANON, etc.
                          / belonging to the local mode field, IXON, ICNTRL, etc. belonging to the input flag field, and so forth).  
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
      printf("%d\r\n", buff); //if so, format byte as decimal number (ASCII code)
    }
    else
    {
      printf("%d ('%c')\r\n", buff, buff);
    }
  } 
  return 0;
}

