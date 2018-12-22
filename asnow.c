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
#include <math.h>

#define ONE_SECOND 1000000
#define RANDF(x) ((float)rand()/((float)RAND_MAX/((float)x)))

typedef struct {
	char	*buffer;
	int	columns;
	int	rows;
	int	size;
} Screen;

typedef struct {
	char	shape;
	int	column;
	float	row;
	int	falling;
	float	speed;
} Snowflake;

char SnowShape[] = {'.', '+', '*', 'x', 'X'};

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
	s->row = 0.0f;
	s->falling = 1;
	s->speed = 0.3f + RANDF(1.3f);
	return s;
}

int
flake_is_blocked(Screen *scr, Snowflake *snow)
{
	int row = (int)floorf(snow->row);

	if (row >= scr->rows - 1)
		return 1;
	if (get_from_screen(scr, snow->column, row+1) != ' ')
		return 1;
	if ((get_from_screen(scr, (snow->column-1) % scr->columns, row+1) | \
	    get_from_screen(scr, (snow->column+1) % scr->columns, row+1)) != ' ')
		return rand() % 2;
	return 0;
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
	srand(time(0));
	Snowflake *snow = flake(scr->columns);
	for (;;) {
		copy_screen(buf, scr);
		put_in_screen(scr, snow->column, (int)floorf(snow->row), snow->shape);
		if (flake_is_blocked(scr, snow)) {
			copy_screen(buf, scr);
			snow->falling = 0;
		}
		draw_screen(scr);
		copy_screen(scr, buf);
		if (snow->falling)
			snow->row += snow->speed;
		else {
			free(snow);
			snow = flake(scr->columns);
		}
		usleep(ONE_SECOND/8);
	}
	return 0;
}
