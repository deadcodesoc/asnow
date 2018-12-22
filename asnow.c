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

#define BLANK		' '
#define ONE_SECOND	1000000
#define RANDF(x)	((float)rand()/((float)RAND_MAX/((float)x)))
#define NELEMS(x)	(sizeof(x)/sizeof((x)[0]))

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
	Screen *s;

	if ((s = malloc(sizeof(Screen))) == NULL) {
		goto err;
	}
	if ((s->buffer = malloc(sizeof(char) * max)) == NULL) {
		goto err2;
	}
	s->columns = columns;
	s->rows = rows;
	s->size = max;
	return s;

 err2:
	free(s);
 err:
	return NULL;
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
merge_screen(Screen *dst, Screen *src)
{
	char *max = dst->buffer + dst->size;
	char *p, *q;
	for (p=dst->buffer, q=src->buffer; p < max; p++, q++)
		if (*q != BLANK)
			*p = *q;
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

void
init_flake(Snowflake *s, int columns)
{
	s->shape = SnowShape[rand() % NELEMS(SnowShape)];
	s->column = RANDF(columns);
	s->row = 0.0f;
	s->falling = 1;
	s->speed = 0.3f + RANDF(1.2f);
	s->phase = RANDF(2.0f * M_PI);
	s->freq = RANDF(0.2f);
	s->wobble = 0.5f + RANDF(3.5f);
}

int
flake_is_blocked(Screen *scr, Snowflake *snow, int column)
{
	int row = (int)floorf(snow->row);
	int speed = (int)ceilf(snow->speed);

	/* Block if we're at the bottom of the screen */
	if (row >= scr->rows - 1) {
		snow->row = scr->rows - 1;
		return 1;
	}

	for (int i = 1; i <= speed; i++) {
		/* Block if we have an obstacle directly below us */
		if (get_from_screen(scr, column, row+i) != BLANK) {
			snow->row += i - 1;
			return 1;
		}

		/* If we have an obstacle in neighboring columns, maybe block */
		if ((get_from_screen(scr, (column-1) % scr->columns, row+i) | \
		    get_from_screen(scr, (column+1) % scr->columns, row+i)) != BLANK)
			return rand() % 2;
  }
  
	return 0;
}

void
melt_flakes(Screen *scr)
{
	char *max = scr->buffer + scr->size;
	for (char *p = scr->buffer; p < max; p++)
		for (int i = 0; i < NELEMS(SnowShape); i++)
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
snowfall(int w, int h, int intensity, char *msg)
{
	Screen *scr, *buf, *bg, *fg;
	useconds_t start;
	useconds_t elapsed;
	float frame_rate = 8.0;

	if ((scr = new_screen(w, h)) == NULL) {
		goto err;
	}
	if ((buf = new_screen(w, h)) == NULL) {
		goto err2;
	}
	if ((bg  = new_screen(w, h)) == NULL) {
		goto err3;
	}
	if ((fg  = new_screen(w, h)) == NULL) {
		goto err4;
	}
	fill_screen(scr, BLANK);
	fill_screen(buf, BLANK);
	fill_screen(bg,  BLANK);
	fill_screen(fg,  BLANK);

	if (msg != NULL) {
		text_screen(bg, (w - strlen(msg)) / 2, h / 2, msg);
	}

	Snowflake *snow = malloc((intensity + 1) * sizeof(Snowflake));
        for (int i = 0; i < intensity; i++) {
		init_flake(&snow[i], scr->columns);
	}

	for (;;) {
		start = now();
		copy_screen(buf, fg);
		for (int i = 0; i < intensity; i++) {
			Snowflake *s = &snow[i];
			int column = (int)floorf(s->column + s->wobble * sinf(s->phase));
			if (flake_is_blocked(bg, s, column)) {
				put_in_screen(bg, column, (int)floorf(s->row), s->shape);
				s->falling = 0;
			}
			if (s->falling)
				put_in_screen(fg, column, (int)floorf(s->row), s->shape);
		}
		for (int i = 0; i < intensity; i++) {
			Snowflake *s = &snow[i];
			if (s->falling) {
				s->row += s->speed;
				s->phase += s->freq;
			} else {
				init_flake(s, scr->columns);
			}
		}
		if (rand() % 1000 == 0)
			melt_flakes(bg);
		copy_screen(scr, bg);
		merge_screen(scr, fg);
		copy_screen(fg, buf);
		draw_screen(scr);
		elapsed = now() - start;
		usleep((ONE_SECOND-elapsed)/frame_rate);
	}
	return 0;

 err4:
	free(bg);
 err3:
	free(buf);
 err2:
	free(scr);
 err:
	return -1;
}

int
main(int argc, char *argv[])
{
	struct winsize ws;
	int intensity = 5;	/* The number of simultaneous snowflakes */

	srand(time(NULL));

	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

	char *msg = argc > 1 ? argv[1] : NULL;
	if (snowfall(ws.ws_col, ws.ws_row, intensity, msg) < 0) {
		fprintf(stderr, "no snow forecast today.\n");
		exit(EXIT_FAILURE);
	}
}
