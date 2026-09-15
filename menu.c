#include "menu.h"
#include "common.h"

#include <menu.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/cdefs.h>

static char *diagnostic_menu_options[] = {
		"Vehicle Info",
		"Trouble Codes",
		"Monitor PIDs",
		"Exit"
};

void diagnosticMenuInit(DiagnosticMenu *dm, obd2_reader_ctx *ctx) {
	dm->done = false;
	dm->ctx = ctx;

	ITEM **menu_items = calloc(5, sizeof(ITEM*));
	// TODO - error handling
	menu_items[0] = new_item("Vehicle Info", NULL);
	menu_items[1] = new_item("Trouble Codes", NULL);
	menu_items[2] = new_item("Monitor PIDs", NULL);
	menu_items[3] = new_item("Exit", NULL);
	menu_items[4] = NULL;

	dm->menu = new_menu(menu_items);
}

void diagnosticMenuLoop(DiagnosticMenu *dm) {
	mvprintw(LINES - 2, 0, "F1 to exit");
	post_menu(dm->menu);
	int c = getch();
	switch (c) {
		case KEY_F(1):
			dm->done = true;
			break;
		case KEY_DOWN:
			dm->curr_choice++;
			dm->curr_choice = min(dm->curr_choice, (int)ARRAY_SIZE(diagnostic_menu_options) - 1);
			menu_driver(dm->menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			dm->curr_choice--;
			dm->curr_choice = max(dm->curr_choice, 0);
			menu_driver(dm->menu, REQ_UP_ITEM);
			break;
		case 10:
			// TODO - handle enter
			switch (dm->curr_choice) {
				case 0:
					unpost_menu(dm->menu);
					diagnosticMenuDisplayVehicleInfo(dm);
					post_menu(dm->menu);
					break;
				case 3:
					dm->done = true;
					break;
				default:
					break;
			}
			break;
	}
	refresh();
}

void diagnosticMenuDisplayVehicleInfo(DiagnosticMenu *dm __attribute_maybe_unused__) {
	wclear(stdscr);
	mvprintw(0, 0, "VEHICLE INFO");
	getch();
	wclear(stdscr);
}
