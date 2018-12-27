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

int
put_on_frame(Frame *frm, int col, int row, char ch)
{
	int pos = row * frm->columns + col;
	if (pos < 0 || pos > frm->size) {
		return -1;
	}
	frm->buffer[pos] = ch;
	return ch;
}

int
get_from_frame(Frame *frm, int col, int row)
{
	int pos = row * frm->columns + col;
	if (pos < 0 || pos > frm->size) {
		return -1;
	}
	return frm->buffer[pos];
}

void
fill_frame(Frame *frm, char ch)
{
	memset(frm->buffer, ch, frm->size);
}

int
text_on_frame(Frame *frm, int col, int row, char *s)
{
	int pos = row * frm->columns + col;
	if (pos < 0 || pos > frm->size) {
		return -1;
	}
	size_t len = MIN(strlen(s),frm->size-pos);
	memcpy(frm->buffer+pos, s, len);
	return len;
}

int
stamp_on_frame(Frame *frm, int col, int row, char *stamp[], int cols, int rows)
{
	int pos = row * frm->columns + col;
	if (pos < 0 || pos > frm->size) {
		return -1;
	}
	for (int i = 0; i < rows && i+row < frm->rows; pos += frm->columns, i++) {
		size_t len = MIN(cols,frm->columns-col);
		memcpy(frm->buffer+pos, stamp[i], len);
	}
	return 0;
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
