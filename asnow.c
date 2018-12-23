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
} Frame;

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

Frame *
new_frame(int columns, int rows)
{
	int max = columns * rows;
	Frame *s;

	if ((s = malloc(sizeof(Frame))) == NULL) {
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
put_on_frame(Frame *frm, int col, int row, char ch)
{
	size_t pos = row * frm->columns + col;
	frm->buffer[pos] = ch;
}

int
get_from_frame(Frame *frm, int col, int row)
{
	size_t pos = row * frm->columns + col;
	return frm->buffer[pos];
}

void
fill_frame(Frame *frm, char ch)
{
	memset(frm->buffer, ch, frm->size);
}

void
text_on_frame(Frame *frm, int col, int row, char *s)
{
	size_t pos = row * frm->columns + col;
	memcpy(frm->buffer+pos, s, strlen(s));
}

void
copy_frame(Frame *dst, Frame *src)
{
	memcpy(dst->buffer, src->buffer, src->size);
}

void
merge_screen(Frame *dst, Frame *src)
{
	char *max = dst->buffer + dst->size;
	char *p, *q;
	for (p=dst->buffer, q=src->buffer; p < max; p++, q++)
		if (*q != BLANK)
			*p = *q;
}

void
draw_screen(Frame *frm)
{
	char *max = frm->buffer + frm->size;
	for (char *p = frm->buffer; p < max; p++)
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
flake_is_blocked(Frame *frm, Snowflake *snow, int column)
{
	int row = (int)floorf(snow->row);
	int speed = (int)ceilf(snow->speed);

	/* Block if we're at the bottom of the screen */
	if (row >= frm->rows - 1) {
		snow->row = frm->rows - 1;
		return 1;
	}

	for (int i = 1; i <= speed; i++) {
		/* Block if we have an obstacle directly below us */
		if (get_from_frame(frm, column, row+i) != BLANK) {
			snow->row += i - 1;
			return 1;
		}

		/* If we have an obstacle in neighboring columns, maybe block */
		if ((get_from_frame(frm, (column-1) % frm->columns, row+i) | \
		    get_from_frame(frm, (column+1) % frm->columns, row+i)) != BLANK)
			return rand() % 2;
	}
  
	return 0;
}

int
flake_is_blocker(Frame *frm, Snowflake *snow)
{
	/* Not a blocker if we're at the top of the screen */
	if (snow->row == 0)
		return 0;

	/* Blocker if we have objects directly above us */
	if (get_from_frame(frm, snow->column, snow->row-1) != BLANK)
		return 1;
	if ((snow->column > 0 && snow->column < frm->columns-1) && \
	     (get_from_frame(frm, snow->column-1, snow->row-1) | \
	      get_from_frame(frm, snow->column+1, snow->row-1)) != BLANK)
		return 1;

	return 0;
}

void
melt_flakes(Frame *frm)
{
	char *max = frm->buffer + frm->size;
	size_t pos = 0;
	Snowflake snow;
	for (char *p = frm->buffer; p < max; p++, pos++) {
		if (*p == BLANK) {
			continue;
		}
		snow.column = pos % frm->columns;
		snow.row    = pos / frm->columns;
		if (flake_is_blocker(frm, &snow)) {
			continue;
		}
		for (int i = 0; i < NELEMS(SnowShape); i++) {
			if (MeltMap[i][0] == *p) {
				*p = MeltMap[i][1];
			}
		}
	}
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
	Frame *scr, *buf, *bg, *fg;
	useconds_t start;
	useconds_t elapsed;
	float frame_rate = 8.0;
	Snowflake *snow;

	if ((scr = new_frame(w, h)) == NULL) {
		goto err;
	}
	if ((buf = new_frame(w, h)) == NULL) {
		goto err2;
	}
	if ((bg  = new_frame(w, h)) == NULL) {
		goto err3;
	}
	if ((fg  = new_frame(w, h)) == NULL) {
		goto err4;
	}
	if ((snow = malloc((intensity + 1) * sizeof(Snowflake))) == NULL) {
		goto err5;
	}

	fill_frame(scr, BLANK);
	fill_frame(buf, BLANK);
	fill_frame(bg,  BLANK);
	fill_frame(fg,  BLANK);

	if (msg != NULL) {
		text_on_frame(bg, (w - strlen(msg)) / 2, h / 2, msg);
	}

        for (int i = 0; i < intensity; i++) {
		init_flake(&snow[i], scr->columns);
	}

	int melt_threshold = (scr->columns * scr->rows) / 7;

	for (;;) {
		start = now();
		copy_frame(buf, fg);
		for (int i = 0; i < intensity; i++) {
			Snowflake *s = &snow[i];
			int column = (int)floorf(s->column + s->wobble * sinf(s->phase));
			if (flake_is_blocked(bg, s, column)) {
				put_on_frame(bg, column, (int)floorf(s->row), s->shape);
				s->falling = 0;
			}
			if (s->falling)
				put_on_frame(fg, column, (int)floorf(s->row), s->shape);
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
		if (rand() % melt_threshold == 0)
			melt_flakes(bg);
		copy_frame(scr, bg);
		merge_screen(scr, fg);
		copy_frame(fg, buf);
		draw_screen(scr);
		elapsed = now() - start;
		usleep((ONE_SECOND-elapsed)/frame_rate);
	}
	return 0;

 err5:
	free(fg);
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
