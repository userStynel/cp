#include "DrawingTools.h"

extern COLORREF gColor;
extern object_polygon* tail_saving_path;

void swaping(int* x, int* y)
{
	int temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

void Drawing_PEN(HDC hdc, int ex_x, int ex_y, int now_x, int now_y)
{
	MoveToEx(hdc, ex_x, ex_y, NULL);
	LineTo(hdc, now_x, now_y);
}

void ADD_PATH(object_polygon* p_poly, int x, int y, COLORREF c, COLORREF d)
{
	polygon_path *searcher = p_poly->path;
	if (p_poly->path == NULL)
	{
		p_poly->path = (polygon_path *)malloc(sizeof(polygon_path));
		p_poly->path->x = x;
		p_poly->path->y = y;
		p_poly->path->prev_colour = c;
		p_poly->path->next_colour = d;
		p_poly->path->next = NULL;
	}
	else
	{
		while (searcher->next != NULL)
		{
			if (searcher->x == x && searcher->y == y)
				return;
			searcher = searcher->next;
		}
		if (searcher->x == x && searcher->y == y)
			return;
		searcher->next = (polygon_path*)malloc(sizeof(polygon_path));
		searcher->next->x = x;
		searcher->next->y = y;
		searcher->next->prev_colour = c;
		searcher->next->next_colour = d;
		searcher->next->next = NULL;
	}
}

void INIT_POLYGON(object_polygon** h_p_poly, object_polygon** t_p_poly)
{
	*h_p_poly = (object_polygon *)malloc(sizeof(object_polygon));
	*t_p_poly = (object_polygon *)malloc(sizeof(object_polygon));

	(*h_p_poly)->prev = *h_p_poly;
	(*h_p_poly)->next = *t_p_poly;
	(*h_p_poly)->path = NULL;

	(*t_p_poly)->prev = *h_p_poly;
	(*t_p_poly)->next = *t_p_poly;
	(*t_p_poly)->path = NULL;
}

void ADD_POLYGON(object_polygon** t_p_poly, object_polygon** recent_node)
{
	object_polygon *new_node;
	new_node = (object_polygon*)malloc(sizeof(object_polygon));
	new_node->next = *t_p_poly;
	new_node->prev = (*t_p_poly)->prev;
	new_node->path = NULL;
	(*t_p_poly)->prev->next = new_node;
	(*t_p_poly)->prev = new_node;
	
	*recent_node = new_node;
}

void UNDO_PEN(object_polygon* p_poly, HWND hwnd)
{
	HDC hdc;
	int x, y;
	polygon_path *searcher = p_poly->path;
	while (searcher != NULL)
	{
		hdc = GetDC(hwnd);
		x = searcher->x;
		y = searcher->y;
		if(x >= 0 && y >= 0)
			SetPixel(hdc, x + 50, y + 10, searcher->prev_colour);
		else
		{
			HBRUSH fillbrush;
			HBRUSH oldbrush;
			fillbrush = CreateSolidBrush(searcher->prev_colour);
			oldbrush = (HBRUSH)SelectObject(hdc, fillbrush);
			ExtFloodFill(hdc, searcher->next->x+50, searcher->next->y+10, searcher->next_colour, FLOODFILLSURFACE);
			SelectObject(hdc, oldbrush);
			DeleteObject(fillbrush);
			break;
		}
		searcher = searcher->next;
		ReleaseDC(hwnd, hdc);
	}
}

void REDO_PEN(object_polygon* p_poly, HWND hwnd)
{
	HDC hdc;
	int x, y;
	polygon_path *searcher = p_poly->path;
	while (searcher != NULL)
	{
		hdc = GetDC(hwnd);
		x = searcher->x;
		y = searcher->y;
		if(x>=0 && y>=0)
			SetPixel(hdc, x + 50, y + 10, searcher->next_colour);
		else
		{
			HBRUSH fillbrush;
			HBRUSH oldbrush;
			fillbrush = CreateSolidBrush(searcher->next_colour);
			oldbrush = (HBRUSH)SelectObject(hdc, fillbrush);
			ExtFloodFill(hdc, searcher->next->x + 50, searcher->next->y + 10, searcher->prev_colour, FLOODFILLSURFACE);
			SelectObject(hdc, oldbrush);
			DeleteObject(hdc);
			break;
		}
		searcher = searcher->next;
		ReleaseDC(hwnd, hdc);
	}
}

void Drawing_b_LINE(HDC hdc, int sx, int sy, int x, int y, COLORREF colour, object_polygon* p_poly, BOOL save, 
	BOOL reverse)
{
	int inc;
	int px, py;
	int dx, dy;
	int d;
	if (abs(x - sx) >= abs(y - sy))
	{
		if (sx > x)
		{
			swaping(&sx, &x);
			swaping(&sy, &y);
		}
		py = sy;

		inc = (y > sy) ? 1 : -1;
		dx = x - sx;
		dy = abs(y - sy);
		d = 2 * dy - dx;
		for (int i = sx; i <= x; i++)
		{
			if (save)
			{
				if (!reverse)
					ADD_PATH(p_poly, i, py, GetPixel(hdc, i, py), colour);
				else
					ADD_PATH(p_poly, i, py, RGB(255, 255, 255) - GetPixel(hdc, i, py), colour);
			}
			SetPixel(hdc, i, py, colour);
			if (d > 0)
			{
				py += inc;
				d += 2 * (dy - dx);
			}
			else
				d += 2 * dy;
		}
	}
	else
	{
		if (sy > y)
		{
			swaping(&sx, &x);
			swaping(&sy, &y);
		}
		px = sx;

		inc = (x > sx) ? 1 : -1;
		dx = abs(x - sx);
		dy = y - sy;
		d = 2 * dx - dy;
		for (int i = sy; i <= y; i++)
		{
			if (save)
			{
				if (!reverse)
					ADD_PATH(p_poly, px, i, GetPixel(hdc, px, i), colour);
				else
					ADD_PATH(p_poly, px, i, RGB(255, 255, 255) - GetPixel(hdc, px, i), colour);
			}
			SetPixel(hdc, px, i, colour);
			if (d > 0)
			{
				px += inc;
				d += 2 * (dx - dy);
			}
			else
				d += 2 * dx;
		}
	}
}

void Drawing_b_RECT(HDC hdc, int sx, int sy, int fx, int fy, COLORREF colour, object_polygon* p_poly, BOOL save,
	BOOL reverse)
{
	if (sx > fx)
	{
		swaping(&sx, &fx);
	}
	if (sy > fy)
	{
		swaping(&sy, &fy);
	}
	if (sx != fx && fy != sy)
	{
		Drawing_b_LINE(hdc, sx, sy, sx, fy, colour, p_poly, save, reverse);
		Drawing_b_LINE(hdc, sx + 1, fy, fx, fy, colour, p_poly, save, reverse);
		Drawing_b_LINE(hdc, fx, fy - 1, fx, sy, colour, p_poly, save, reverse);
		Drawing_b_LINE(hdc, fx - 1, sy, sx+1, sy, colour, p_poly, save, reverse);
	}
}

void Drawing_b_CIRCLE(HDC hdc, int cx, int cy, int radius, COLORREF colour, object_polygon* p_poly, BOOL save,
	BOOL reverse)
{
	int h = 1-radius;
	int x = 0;
	int y = radius;
	int deltaE = 3;
	int deltaNE = 5 - 2 * radius;
	while (y >= x)
	{
		if (save)
		{
			ADD_PATH(p_poly, cx + x, cy + y, RGB(255, 255, 255) - GetPixel(hdc, cx + x, cy + y), colour);
			ADD_PATH(p_poly, cx + y, cy + x, RGB(255, 255, 255) - GetPixel(hdc, cx + y, cy + x), colour);
			ADD_PATH(p_poly, cx + x, cy - y, RGB(255, 255, 255) - GetPixel(hdc, cx + x, cy - y), colour);
			ADD_PATH(p_poly, cx - y, cy + x, RGB(255, 255, 255) - GetPixel(hdc, cx - y, cy + x), colour);
			ADD_PATH(p_poly, cx + y, cy - x, RGB(255, 255, 255) - GetPixel(hdc, cx + y, cy - x), colour);
			ADD_PATH(p_poly, cx - x, cy - y, RGB(255, 255, 255) - GetPixel(hdc, cx - x, cy - y), colour);
			ADD_PATH(p_poly, cx - y, cy - x, RGB(255, 255, 255) - GetPixel(hdc, cx - y, cy - x), colour);
			ADD_PATH(p_poly, cx - x, cy + y, RGB(255, 255, 255) - GetPixel(hdc, cx - x, cy + y), colour);
		}
		SetPixel(hdc, cx + x, cy + y, colour);
		SetPixel(hdc, cx + x, cy - y, colour);
		if (x != y)
		{
			SetPixel(hdc, cx + y, cy + x, colour);
			SetPixel(hdc, cx - y, cy + x, colour);
		}
		if (x != 0)
		{
			SetPixel(hdc, cx - x, cy + y, colour);
			SetPixel(hdc, cx - x, cy - y, colour);
		}
		if (x != y && x != 0)
		{
			SetPixel(hdc, cx + y, cy - x, colour);
			SetPixel(hdc, cx - y, cy - x, colour);
		}
		if (h < 0)
		{
			h += deltaE;
			deltaE += 2;
			deltaNE += 2;
		}
		else
		{
			h += deltaNE;
			deltaE += 2;
			deltaNE += 4;
			y--;
		}
		x++;
	}
}

void Deleting_PATH(object_polygon* p_poly)
{
	polygon_path* deleting_path=p_poly->path;
	polygon_path* temp;
	while (1)
	{
		temp = deleting_path->next;
		free(deleting_path);
		if (temp == NULL)
			return;
		deleting_path = temp;
	}
}

void Deleting_After(object_polygon* node)
{
	object_polygon *deleting=node->next;
	object_polygon *temp;
	while (1)
	{
		temp = deleting->next;
		Deleting_PATH(deleting);
		free(deleting);
		if (temp == tail_saving_path)
			break;
		deleting = temp;
	}
	node->next = tail_saving_path;
	tail_saving_path->prev = node;
}

int getRadius(int x1, int y1, int x2, int y2)
{
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}