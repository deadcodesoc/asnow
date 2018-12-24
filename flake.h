#ifndef ASNOW_FLAKE_H_
#define ASNOW_FLAKE_H_

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

extern char SnowShape[5];

void       flake_init(Snowflake *, int);

#endif  /* ASNOW_FLAKE_H_ */
