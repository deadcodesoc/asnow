#include "frame.h"
	
void
circle_on_frame(Frame *frm, int col0, int row0, int rad, char ch)
{
	int col = rad-1;
	int row = 0;
	int drow = 1;
	int dcol = 1;
	int eps  = dcol - (rad << 1);

	while (col >= row) {
		put_on_frame(frm, col0+col, row0+row, ch);
		put_on_frame(frm, col0+row, row0+col, ch);
		put_on_frame(frm, col0-row, row0+col, ch);
		put_on_frame(frm, col0-col, row0+row, ch);
		put_on_frame(frm, col0-col, row0-row, ch);
		put_on_frame(frm, col0-row, row0-col, ch);
		put_on_frame(frm, col0+row, row0-col, ch);
		put_on_frame(frm, col0+col, row0-row, ch);

		if (eps <= 0) {
			row++;
			eps += drow;
			drow += 2;
		}
		if (eps > 0) {
			col--;
			dcol += 2;
			eps += dcol - (rad << 1);
		}
	}
}
