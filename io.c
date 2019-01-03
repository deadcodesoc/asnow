#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

static struct termios saved_tios;

void
prepare_term()
{
	struct termios tios;
	tcgetattr(0, &saved_tios);
	tios = saved_tios;
	tios.c_lflag &= ~ECHO + ~ICANON;
	tcsetattr(0, TCSANOW, &tios);
}

void
restore_term()
{
	tcsetattr(0, TCSANOW, &saved_tios);
}

int
getch()
{
	char buf[1];
	struct timeval tv = {0, 0};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	int nfds = select(1, &fds, NULL, NULL, &tv);
	if (nfds == 1) {
		if (read(0, &buf, 1) != 1) {
			return -1;
		}
		return buf[0];
	}
	return -1;
}
