#ifndef ASNOW_FLAKE_H_
#define ASNOW_FLAKE_H_

#include <stddef.h>

#define MAX_SNOW 100		/* Max simultaneous flakes */

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

typedef struct {
	size_t size;
	size_t used;
	Snowflake *flake;
} Snow;


extern char SnowShape[5];

void	flake_init(Snowflake *, int);
Snow	*snow_start(const int, const int);
void	snow_end(Snow *);

#endif  /* ASNOW_FLAKE_H_ */
