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

Frame*     new_frame(int columns, int rows);
void       put_on_frame(Frame *frm, int col, int row, char ch);
int        get_from_frame(Frame *frm, int col, int row);
void       fill_frame(Frame *frm, char ch);
void       text_on_frame(Frame *frm, int col, int row, char *s);
void       copy_frame(Frame *dst, Frame *src);
void       merge_frame(Frame *dst, Frame *src);
void       draw_frame(Frame *frm);
void       init_flake(Snowflake *s, int columns);
int        flake_is_blocked(Frame *frm, Snowflake *snow, int column);
int        flake_is_blocker(Frame *frm, Snowflake *snow);
void       melt_flakes(Frame *frm);
useconds_t now(void);
int        snowfall(int w, int h, int intensity, char *msg);
