/*
 * asnow.c - Snowflakes in term (inspired by xsnow)
 *
 * Rudá Moura <ruda.moura@gmail.com>
 * December, 2018. Public Domain
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>

#define ONE_SECOND 1000000

typedef struct {
	char	*buffer;
	int	columns;
	int	rows;
	int	size;
} Screen;

typedef struct {
	char	shape;
	int	column;
	int	row;
	int	falling;
} Snowflake;

char SnowShape[] = {'.', '+', '*', 'x', 'X'};
char *MeltMap[]  = {". ", "+.", "*+", "Xx", "x."};

Screen *
new_screen(int columns, int rows)
{
	int max = columns * rows;
	Screen *s = malloc(sizeof(Screen));
	s->buffer = malloc(sizeof(char) * max);
	s->columns = columns;
	s->rows = rows;
	s->size = max;
	return s;
}

void
put_in_screen(Screen *scr, int col, int row, char ch)
{
	size_t pos = row * scr->columns + col;
	scr->buffer[pos] = ch;
}

int
get_from_screen(Screen *scr, int col, int row)
{
	size_t pos = row * scr->columns + col;
	return scr->buffer[pos];
}

void
fill_screen(Screen *scr, char ch)
{
	memset(scr->buffer, ch, scr->size);
}

void
text_screen(Screen *scr, int col, int row, char *s)
{
	size_t pos = row * scr->columns + col;
	memcpy(scr->buffer+pos, s, strlen(s));
}

void
copy_screen(Screen *dst, Screen *src)
{
	memcpy(dst->buffer, src->buffer, src->size);
}

void
draw_screen(Screen *scr)
{
	char *max = scr->buffer + scr->size;
	for (char *p = scr->buffer; p < max; p++)
		putchar(*p);
	putchar('\r');
	fflush(stdout);
}

Snowflake*
flake(int columns)
{
	Snowflake *s = malloc(sizeof(Snowflake));
	s->shape = SnowShape[rand() % 5];
	s->column = rand() % columns;
	s->row = 0;
	s->falling = 1;
	return s;
}

int
flake_is_blocked(Screen *scr, Snowflake *snow)
{
	if (snow->row == scr->rows - 1)
		return 1;
	if (get_from_screen(scr, snow->column, snow->row+1) != ' ')
		return 1;
	if ((get_from_screen(scr, (snow->column-1) % scr->columns, snow->row+1) | \
	    get_from_screen(scr, (snow->column+1) % scr->columns, snow->row+1)) != ' ')
		return rand() % 2;
	return 0;
}

void
melt_flakes(Screen *scr)
{
	char *max = scr->buffer + scr->size;
	for (char *p = scr->buffer; p < max; p++)
		for (int i = 0; i < 5; i++)
			if (MeltMap[i][0] == *p)
				*p = MeltMap[i][1];
}

int
main(int argc, char *argv[])
{
	Screen *scr, *buf;
	struct winsize ws;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
	scr = new_screen(ws.ws_col, ws.ws_row);
	buf = new_screen(ws.ws_col, ws.ws_row);
	fill_screen(scr, ' ');
	if (argc > 1)
		text_screen(scr, (ws.ws_col - strlen(argv[1]) ) / 2, ws.ws_row / 2, argv[1]);
	srand(time(0));
	Snowflake *snow = flake(scr->columns);
	for (;;) {
		copy_screen(buf, scr);
		put_in_screen(scr, snow->column, snow->row, snow->shape);
		if (flake_is_blocked(scr, snow)) {
			copy_screen(buf, scr);
			snow->falling = 0;
		}
		draw_screen(scr);
		copy_screen(scr, buf);
		if (snow->falling)
			snow->row = (snow->row + 1) % scr->rows;
		else {
			free(snow);
			snow = flake(scr->columns);
		}
		if (rand() % 1000 == 0)
			melt_flakes(scr);
		usleep(ONE_SECOND/8);
	}
	return 0;
}
