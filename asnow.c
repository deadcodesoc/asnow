/*
 * (ASCII) Snowflakes in terminal (inspired by xsnow)
 *
 * The asnow Authors
 * December, 2018. Public Domain
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
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
	float	column;
	float	row;
	int	falling;
	float	speed;		/* falling speed */
	float	phase;		/* wobble phase */
	float	freq;		/* wobble frequency */
	float	wobble;		/* wobble amount */
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
	s->column = RANDF(columns);
	s->row = 0.0f;
	s->falling = 1;
	s->speed = 0.3f + RANDF(1.3f);
	s->phase = RANDF(2.0f * M_PI);
	s->freq = RANDF(0.2f);
	s->wobble = RANDF(1.3f);
	return s;
}

int
flake_is_blocked(Screen *scr, Snowflake *snow)
{
	int row = (int)floorf(snow->row);
	int column = (int)floorf(snow->column);

	if (row >= scr->rows - 1) {
		snow->row = scr->rows - 1;
		return 1;
	}
	if (get_from_screen(scr, column, row+1) != ' ')
		return 1;
	if ((get_from_screen(scr, (column-1) % scr->columns, row+1) | \
	    get_from_screen(scr, (column+1) % scr->columns, row+1)) != ' ')
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

useconds_t
now(void)
{
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return tval.tv_sec * ONE_SECOND + tval.tv_usec;
}

int
main(int argc, char *argv[])
{
	Screen *scr, *buf;
	struct winsize ws;
	int intensity = 5;	/* The number of simultaneous snowflakes */
	useconds_t start;
	useconds_t elapsed;
	float frame_rate = 8.0;

	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
	scr = new_screen(ws.ws_col, ws.ws_row);
	buf = new_screen(ws.ws_col, ws.ws_row);
	fill_screen(scr, ' ');
	if (argc > 1)
		text_screen(scr, (ws.ws_col - strlen(argv[1]) ) / 2, ws.ws_row / 2, argv[1]);
	srand(time(0));

	Snowflake **snow = malloc((intensity + 1) * sizeof(Snowflake *));
        int i = 0;
	while (i < intensity) {
		snow[i++] = flake(scr->columns);
	}
	snow[i] = NULL;

	for (;;) {
		start = now();
		copy_screen(buf, scr);
		for (Snowflake **s = snow; *s; s++) {
			if (flake_is_blocked(scr, *s)) {
				put_in_screen(scr, (int)floorf((*s)->column), (int)floorf((*s)->row), (*s)->shape);
				copy_screen(buf, scr);
				(*s)->falling = 0;
			}
		}
		for (Snowflake **s = snow; *s; s++) {
			put_in_screen(scr, (int)floorf((*s)->column), (int)floorf((*s)->row), (*s)->shape);
		}
		draw_screen(scr);
		copy_screen(scr, buf);
		for (Snowflake **s = snow; *s; s++) {
			if ((*s)->falling) {
				(*s)->row += (*s)->speed;
				(*s)->column += (*s)->wobble * sinf((*s)->phase);
				(*s)->phase += (*s)->freq;
			} else {
				free(*s);
				*s = flake(scr->columns);
			}
		}
		if (rand() % 1000 == 0)
			melt_flakes(scr);
		elapsed = now() - start;
		usleep((ONE_SECOND-elapsed)/frame_rate);
	}
	return 0;
}
