/*
 * (ASCII) Snowflakes in terminal (inspired by xsnow)
 *
 * The asnow Authors
 * December, 2018. Public Domain
 *
 */


#include "asnow.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <errno.h>
#include "stamp.h"

extern int optind;

char *MeltMap[]  = {". ", "+.", "*+", "Xx", "x."};


int
flake_is_blocked(Frame *frm, Snowflake *snow, int column)
{
	int row = (int)floorf(snow->row);
	int speed = (int)ceilf(snow->speed);

	/* Block if we're at the bottom of the frame */
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
	/* Not a blocker if we're at the top of the frame */
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
	Snow *snow;

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
	if ((snow = snow_start(intensity, scr->columns)) == NULL) {
		goto err5;
	}

	fill_frame(scr, BLANK);
	fill_frame(buf, BLANK);
	fill_frame(bg,  BLANK);
	fill_frame(fg,  BLANK);

	int ntrees = (scr->columns * scr->rows) / 500;

	for (int i=0; i < ntrees; i++) {
		int col = rand() % (bg->columns-strlen(stamp_small_tree[0]));
		int row = rand() % (bg->rows-NELEMS(stamp_small_tree));
		stamp_on_frame(bg, col, row, stamp_small_tree, strlen(stamp_small_tree[0]), NELEMS(stamp_small_tree));
	}

	if (msg != NULL) {
		text_on_frame(bg, (w - strlen(msg)) / 2, h / 2, msg);
	}

	int melt_threshold = (scr->columns * scr->rows) / 7;

	for (;;) {
		start = now();
		copy_frame(buf, fg);
		for (int i = 0; i < intensity; i++) {
			Snowflake *s = &snow->flake[i];
			int column = (int)floorf(s->column + s->wobble * sinf(s->phase));
			if (flake_is_blocked(bg, s, column)) {
				s->falling = 0;
				if (s->row != 0) {
					put_on_frame(bg, column, (int)floorf(s->row), s->shape);
				}
			}
			if (s->falling) {
				put_on_frame(fg, column, (int)floorf(s->row), s->shape);
				s->row += s->speed;
				s->phase += s->freq;
			} else {
				flake_init(s, scr->columns);
			}
		}
		if (rand() % melt_threshold == 0)
			melt_flakes(bg);
		copy_frame(scr, bg);
		merge_frame(scr, fg);
		copy_frame(fg, buf);
		draw_frame(scr);
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

static void usage(const char *cmd)
{
	printf("Usage: %s [options] [message]\n", cmd);
	printf("Available options:\n"
"    -i --intensity num   Set the snowfall intensity (default 5)\n"
"    -h --help            Display a summary of the command line options\n"
"    -v --version         Print version information and exit\n"
	);
}

static const struct option lopt[] = {
	{ "help",               0, 0, 'h' },
	{ "intensity",          1, 0, 'i' },
	{ "version",            0, 0, 'v' },
	{ NULL,                 0, 0, 0   }
};

int
main(int argc, char *argv[])
{
	struct winsize ws;
	int intensity = 5;	/* The number of simultaneous snowflakes */
	int optidx = 0;
	int o;

#define OPTIONS "hi:v"
	while ((o = getopt_long(argc, argv, OPTIONS, lopt, &optidx)) != -1) {
		switch (o) {
		case 'i':
			intensity = strtoul(optarg, NULL, 0);
			break;
		case 'v':
			printf("asnow " VERSION "\n");
			exit(EXIT_SUCCESS);
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		default:
			exit(EXIT_FAILURE);
		}
	}

	srand(time(NULL));

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0) {
		perror(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Build optional message */
	int msglen = 0;
	char *msg;
	for (int i = optind; i < argc; i++) {
		msglen += strlen(argv[i]) + 1;
	}
	if ((msg = (char *)malloc(msglen)) == NULL) {
		perror(argv[0]);
		exit(EXIT_FAILURE);
	}
	*msg = 0;
	if (optind < argc) {
		strncat(msg, argv[optind], msglen);
		for (int i = optind + 1; i < argc; i++) {
			strncat(msg, " ", msglen);
			strncat(msg, argv[i], msglen);
		}
	}

	if (snowfall(ws.ws_col, ws.ws_row, intensity, msg) < 0) {
		fprintf(stderr, "no snow forecast today.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "you shouldn't see this message\n");
}
