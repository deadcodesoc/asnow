#include <stdlib.h>
#include "frame.h"

static void
line_low(Frame *frm, int col0, int row0, int col1, int row1, char ch)
{
	int dcol = col1 - col0;
	int drow = row1 - row0;
	int row  = row0;
	int eps  = 0;
	int step = 1;
	if (drow < 0) {
		step = -1;
		drow = abs(drow);
	}
	for (int col = col0; col <= col1; col++) {
		put_on_frame(frm, col, row, ch);
		eps += drow;
		if ((eps << 1) >= dcol) {
			row += step;
			eps -= dcol;
		}
	}
}

static void
line_high(Frame *frm, int col0, int row0, int col1, int row1, char ch)
{
	int dcol = col1 - col0;
	int drow = row1 - row0;
	int col  = col0;
	int eps  = 0;
	int step = 1;
	if (dcol < 0) {
		step = -1;
		dcol = abs(dcol);
	}
	for (int row = row0; row <= row1; row++) {
		put_on_frame(frm, col, row, ch);
		eps += dcol;
		if ((eps << 1) >= drow) {
			col += step;
			eps -= drow;
		}
	}
}

void
line_on_frame(Frame *frm, int col0, int row0, int col1, int row1, char ch)
{
	int dcol = col1 - col0;
	int drow = row1 - row0;
	if (abs(drow) < abs(dcol)) {
		if (col0 < col1) {
			line_low(frm, col0, row0, col1, row1, ch);
		} else {
			line_low(frm, col1, row1, col0, row0, ch);
		}
	} else {
		if (row0 < row1) {
			line_high(frm, col0, row0, col1, row1, ch);
		} else {
			line_high(frm, col1, row1, col0, row0, ch);
		}
	}
}
