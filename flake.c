#include "flake.h"
#include <stdlib.h>
#include <math.h>
#include "asnow.h"


char SnowShape[5] = {'.', '+', '*', 'x', 'X'};

void
flake_init(Snowflake *s, int columns)
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

Snow *
snow_start(const int intensity, const int columns)
{
	Snow *snow = (Snow *)malloc(sizeof (Snow));
	snow->flake = (Snowflake *)malloc(MAX_SNOW * sizeof(Snowflake));
	snow->size = MAX_SNOW;
	snow->used = intensity;

	for (int i = 0; i < snow->used; i++) {
		flake_init(&snow->flake[i], columns);
	}

	return snow;
}

void
snow_end(Snow *snow)
{
	free(snow->flake);
	snow->size = 0;
	snow->used = 0;
}

