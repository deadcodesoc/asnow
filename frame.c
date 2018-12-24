/*
 * (ASCII) Snowflakes in terminal (inspired by xsnow)
 *
 * The asnow Authors
 * December, 2018. Public Domain
 *
 */

#include "frame.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Frame *
new_frame(int columns, int rows)
{
	int max = columns * rows;
	Frame *frm;

	if ((frm = malloc(sizeof(Frame))) == NULL) {
		goto err;
	}
	if ((frm->buffer = malloc(sizeof(char) * max)) == NULL) {
		goto err2;
	}
	frm->columns = columns;
	frm->rows    = rows;
	frm->size    = max;
	return frm;

 err2:
	free(frm);
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
stamp_on_frame(Frame *frm, int col, int row, char *stamp[], int cols, int rows)
{
	size_t pos = row * frm->columns + col;
	for (int i = 0; i < rows && i+row < frm->rows; pos += frm->columns, i++) {
		size_t len = MIN(cols,frm->columns-col);
		memcpy(frm->buffer+pos, stamp[i], len);
	}
}

void
copy_frame(Frame *dst, Frame *src)
{
	memcpy(dst->buffer, src->buffer, src->size);
}

void
merge_frame(Frame *dst, Frame *src)
{
	char *max = dst->buffer + dst->size;
	char *p, *q;
	for (p=dst->buffer, q=src->buffer; p < max; p++, q++)
		if (*q != BLANK)
			*p = *q;
}


void
draw_frame(Frame *frm)
{
	char *max = frm->buffer + frm->size;
	for (char *p = frm->buffer; p < max; p++)
		putchar(*p);
	putchar('\r');
	fflush(stdout);
}
