#ifndef ASNOW_H_
#define ASNOW_H_

/*
 * (ASCII) Snowflakes in terminal (inspired by xsnow)
 *
 * The asnow Authors
 * December, 2018. Public Domain
 *
 */

#include <unistd.h>
#include "frame.h"
#include "flake.h"

#define ONE_SECOND	1000000
#define RANDF(x)	((float)rand()/((float)RAND_MAX/((float)x)))
#define NELEMS(x)	(sizeof(x)/sizeof((x)[0]))

int        flake_is_blocked(Frame *frm, Snowflake *snow, int column);
int        flake_is_blocker(Frame *frm, Snowflake *snow);
void       melt_flakes(Frame *frm);
useconds_t now(void);
int        snowfall(int w, int h, float frame_rate, int intensity, float temperature, char *msg);

#endif  /* ASNOW_H_ */
