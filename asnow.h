#ifndef ASNOW_H_
#define ASNOW_H_

/*
 * (ASCII) Snowflakes in terminal (inspired by xsnow)
 *
 * The asnow Authors
 * December, 2018. Public Domain
 *
 */

#include "frame.h"
#include <unistd.h>

#define ONE_SECOND	1000000
#define RANDF(x)	((float)rand()/((float)RAND_MAX/((float)x)))
#define NELEMS(x)	(sizeof(x)/sizeof((x)[0]))

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

void       init_flake(Snowflake *s, int columns);
int        flake_is_blocked(Frame *frm, Snowflake *snow, int column);
int        flake_is_blocker(Frame *frm, Snowflake *snow);
void       melt_flakes(Frame *frm);
useconds_t now(void);
int        snowfall(int w, int h, int intensity, char *msg);

#endif  /* ASNOW_H_ */
