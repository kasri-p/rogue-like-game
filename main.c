#include "menu.h"
#include <locale.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

void draw_border()
{
	for (int i = 1; i < COLS - 1; i++)
	{
		mvprintw(1, i, "█");
		mvprintw(LINES - 2, i, "█");
	}
	for (int i = 2; i < LINES - 2; i++)
	{
		mvprintw(i, 1, "█");
		mvprintw(i, COLS - 2, "█");
	}

	mvprintw(1, 1, "╔");
	mvprintw(1, COLS - 2, "╗");
	mvprintw(LINES - 2, 1, "╚");
	mvprintw(LINES - 2, COLS - 2, "╝");
}

void animate_blood_drip(int frame)
{
	static int drip_positions[] = {0, 0, 0, 0, 0};
	static int drip_speeds[] = {3, 5, 4, 6, 2};
	const int drip_start_x = COLS / 2 - 10;
	const int drip_start_y = LINES / 2 - 4;

	int drip_points[] = {drip_start_x + 2, drip_start_x + 6, drip_start_x + 10, drip_start_x + 14, drip_start_x + 18};

	for (int i = 0; i < 5; i++)
	{
		if (frame % drip_speeds[i] == 0)
		{
			drip_positions[i]++;
			if (drip_positions[i] > 10)
				drip_positions[i] = 0;
		}

		if (drip_positions[i] > 0)
		{
			attron(COLOR_PAIR(1));
			mvprintw(drip_start_y + 1 + drip_positions[i], drip_points[i], "|");
			attroff(COLOR_PAIR(1));
		}
	}
}

void animate_title_blood(int frame)
{
	const int drip_start_x = COLS / 2 - 10;
	const int drip_start_y = LINES / 2 - 4;
	static int title_drip_positions[] = {0, 0, 0, 0, 0};

	int drip_points[] = {drip_start_x + 2,
						 drip_start_x + 6,
						 drip_start_x + 10,
						 drip_start_x + 14,
						 drip_start_x + 18};

	for (int i = 0; i < 5; i++)
	{
		if (frame % (rand() % 5 + 1) == 0)
		{
			title_drip_positions[i]++;
			if (title_drip_positions[i] > 4)
				title_drip_positions[i] = 0;
		}

		if (title_drip_positions[i] > 0)
		{
			attron(COLOR_PAIR(1));
			mvprintw(drip_start_y + title_drip_positions[i], drip_points[i], "|");
			attroff(COLOR_PAIR(1));
		}
	}
}

int main()
{
	setlocale(LC_ALL, "");
	initscr();
	start_color();
	noecho();
	curs_set(0);
	initialize_database();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_WHITE, COLOR_BLACK);

	clear();
	int frame = 0;

	music();

	while (1)
	{
		clear();

		draw_border();

		const char *title_art[] = {
			"╔════════════════════════════════════════════╗",
			"║ ██████╗  ██████╗  ██████╗ ██╗   ██╗███████╗║",
			"║ ██╔══██╗██╔═══██╗██╔════╝ ██║   ██║██╔════╝║",
			"║ ██████╔╝██║   ██║██║  ███╗██║   ██║█████╗  ║",
			"║ ██╔══██╗██║   ██║██║   ██║██║   ██║██╔══╝  ║",
			"║ ██║  ██║╚██████╔╝╚██████╔╝╚██████╔╝███████╗║",
			"║ ╚═╝  ╚═╝ ╚═════╝  ╚═════╝  ╚═════╝ ╚══════╝║",
			"╚════════════════════════════════════════════╝"};

		attron(COLOR_PAIR(1) | A_BOLD);
		for (int i = 0; i < 8; i++)
		{
			mvprintw(LINES / 2 - 4 + i, COLS / 2 - 22, "%s", title_art[i]);
		}
		attroff(COLOR_PAIR(1) | A_BOLD);

		animate_blood_drip(frame);
		animate_title_blood(frame);

		attron(COLOR_PAIR(1) | A_BLINK);
		mvprintw(LINES / 2 + 8, COLS / 2 - 15, "PRESS ANY KEY TO ENTER THE ABYSS...");
		attroff(COLOR_PAIR(1) | A_BLINK);

		refresh();

		nodelay(stdscr, TRUE);
		if (getch() != ERR)
		{
			break;
		}

		usleep(100000);
		frame++;
	}
	nodelay(stdscr, FALSE);
	signup_login();

	endwin();
	return 0;
}