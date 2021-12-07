#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
using namespace std;
int main() {
  char tty[L_ctermid+1] = {0};
  ctermid(tty);
  cout << "ID: " << tty << '\n';
  int fd = ::open(tty, O_RDONLY);
  if (fd < 0) perror("Could not open terminal");
  else {
    cout << "Opened terminal\n";
    struct termios term;
    int r = tcgetattr(fd, &term);
    if (r < 0) perror("Could not get attributes");
    else cout << "Got attributes\n";
  }
  if (isatty(fileno(stdin))) cout << "Is a terminal\n";
  else cout << "Is not a terminal\n";
  struct stat stats;
  int r = fstat(fileno(stdin), &stats);
  if (r < 0) perror("fstat failed");
  else {
    if (S_ISCHR(stats.st_mode)) cout << "S_ISCHR\n";
    else if (S_ISFIFO(stats.st_mode)) cout << "S_ISFIFO\n";
    else if (S_ISREG(stats.st_mode)) cout << "S_ISREG\n";
    else cout << "unknown stat mode\n";
  }
  return 0;
}
