#include "menu.h"
#include "common.h"
#include "obd2.h"

#include <curses.h>
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

	dm->curr_choice = 0;

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
				case 2: // Monitor PIDs
					unpost_menu(dm->menu);
					diagnosticMenuDisplayPidMenu(dm);
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

void diagnosticMenuDisplayPidMenu(DiagnosticMenu *dm __attribute_maybe_unused__) {
	// Make left-half window for menu
	WINDOW *menu_window = newwin(LINES, COLS / 2, 0, 0);
	box(menu_window, 0, 0);
	WINDOW *info_window = newwin(LINES, COLS / 2 - 1, 0, COLS / 2 - 1);
	box(info_window, 0, 0);
	init_pair(1, COLOR_BLUE, COLOR_RED);

	wclear(stdscr);
	ITEM **menu_items = calloc(0xC8, sizeof(ITEM*));
	int idx = 0;
	for (int i = 0; i <= 0xC8; i++) {
		if (obd2_pid_descriptions[i]) {
			menu_items[idx] = new_item(obd2_pid_descriptions[i], NULL);
			idx++;
		}
	}

	MENU *pid_menu = new_menu(menu_items);

	set_menu_win(pid_menu, menu_window);
	set_menu_sub(pid_menu, derwin(menu_window, LINES - 1, COLS / 2 - 1, 0, 0));

	refresh();
	post_menu(pid_menu);
	wrefresh(menu_window);
	wbkgd(info_window, COLOR_PAIR(1));
	if (dm->ctx->pids[0].supported) {
		mvwprintw(info_window, 0, 0, "SUPPORTED");
	} else {
		mvwprintw(info_window, 0, 0, "NOT SUPPORTED");
	}
	wrefresh(info_window);

	int curr_choice = 0;
	bool brk = false;
	while (!brk) {
		// TODO - the SUPPORTED/UNSUPPORTED logic gets buggy due to some of the indices 
		// in the supported_pids array being missing. Need to handle this
		int c = getch();
		werase(menu_window);
		werase(info_window);
		switch (c) {
			case KEY_F(1):
				brk = true;
				break;
			case KEY_DOWN:
				curr_choice = min(curr_choice + 1, idx - 2);
				menu_driver(pid_menu, REQ_DOWN_ITEM);
				diagnosticMenuDisplayPidData(dm, info_window, curr_choice);
				break;
			case KEY_UP:
				curr_choice--;
				curr_choice = max(curr_choice, 0);
				menu_driver(pid_menu, REQ_UP_ITEM);
				diagnosticMenuDisplayPidData(dm, info_window, curr_choice);
				break;
			case 10:
				// TODO - handle enter
				break;
		}

		wrefresh(menu_window);

		wrefresh(info_window);

		refresh();
	}

	unpost_menu(pid_menu);
	//wclear(stdscr);
}

void diagnosticMenuDisplayPidData(DiagnosticMenu *dm __attribute_maybe_unused__, WINDOW *info_window, int pid) {
	if (pid > 147 && pid < 151) {
		pid += 4;
	} else if (pid > 168 && pid < 191) {
		pid += 23;
	} else if (pid > 191 && pid < 194) {
		pid += 3;
	}
	if (pid == 5) {
		diagnosticMenuDisplayCoolantTemp(dm, info_window);
	} else if (pid == 3) {
		diagnosticMenuDisplayFuelSystemStatus(dm, info_window);
	} else if (dm->ctx->pids[pid].supported) {
		mvwprintw(info_window, 0, 0, "SUPPORTED - NOT IMPLEMENTED");
	} else {
		mvwprintw(info_window, 0, 0, "NOT SUPPORTED");
	}
}

void diagnosticMenuDisplayCoolantTemp(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t coolant_temp = hex_chars_to_u8(dm->ctx->pids[0x05].data) - 40;
	mvwprintw(info_window, 0, 0, "COOLANT TEMP: %d C", coolant_temp);
}

void diagnosticMenuDisplayFuelSystemStatus(DiagnosticMenu *dm, WINDOW *info_window) {
	uint16_t fuel_system_status = hex_chars_to_u16(dm->ctx->pids[0x03].data);
	for (int i = 0; i < 2; i++) {
		switch (fuel_system_status << (1 - i)) {
			case 0:
				mvwprintw(info_window, i, 0, "System %d: Motor off", i);
				break;
			case 1:
				mvwprintw(info_window, i, 0, "System %d: Open loop due to insufficient engine temperature", i);
				break;
			case 2:
				mvwprintw(info_window, i, 0, "System %d: Closed loop, using oxygen sensor feedback to determine fuel mix", i);
				break;
			case 4:
				mvwprintw(info_window, i, 0, "System %d: Open loop due to engine load OR fuel cut due to deceleration", i);
				break;
			case 8:
				mvwprintw(info_window, i, 0, "System %d: Open loop due to system failure", i);
				break;
			case 16:
				mvwprintw(info_window, i, 0, "System %d: Closed loop, using at least one oxygen sensor but there is a fault in the feedback system", i);
				break;
			default:
				mvwprintw(info_window, i, 0, "System %d: Fuel system status unknown", i);
		}
	}
}
