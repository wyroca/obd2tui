#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#include <ncurses.h>
#include <menu.h>

#include "obd2.h"
#include "common.h"

void vehicle_info() {
	wclear(stdscr);
	mvprintw(0, 0, "VEHICLE INFO");
	getch();
	wclear(stdscr);
}

int main(int argc, char *argv[]) {
//	struct sigaction sa;
//	sa.sa_handler = handle_exit;
//	sigemptyset(&sa.sa_mask);
//	sa.sa_flags = SA_RESTART;
//	if (sigaction(SIGINT, &sa, NULL) == -1) {
//		printf("Failed to set SIGKILL signal for handling curses de-init\n");
//		return 1;
//	}
	char *menu_options[] = {
		"Vehicle Info",
		"Trouble Codes",
		"Monitor PIDs",
		"Exit"
	};

	obd2_reader_ctx ctx;

	initscr();	
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	ITEM **items = calloc(5, sizeof(ITEM*))	;
	for (int i = 0; i < 4; i++) {
		//items[i] = new_item(menu_options[i], menu_options[i]);
		items[i] = new_item(menu_options[i], NULL);
	}
	items[4] = NULL;

	MENU *menu = new_menu(items);
	mvprintw(LINES - 2, 0, "F1 to exit");
	post_menu(menu);
	refresh();
	int c, choice = 0;
	bool exit = false;
	while (!exit) {
		c = getch();
		switch (c) {
			case KEY_F(1):
				exit = true;
				break;
			case KEY_DOWN:
				choice++;
				choice = min(choice, ARRAY_SIZE(menu_options) - 1);
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				choice--;
				choice = max(choice, 0);
				menu_driver(menu, REQ_UP_ITEM);
				break;
			case 10:
				// TODO - handle enter
				switch (choice) {
					case 0:
						unpost_menu(menu);
						vehicle_info();
						post_menu(menu);
						break;
					case 3:
						exit = true;
						break;
					default:
						break;
				}
				break;
		}
		refresh();
	}

	endwin();

	return 0;
}
