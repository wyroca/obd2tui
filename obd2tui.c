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

#include "menu.h"
#include "obd2.h"
#include "common.h"

int main() {
	obd2_reader_ctx ctx;
	obd2_ctx_init(&ctx);

	initscr();	
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	refresh();

	DiagnosticMenu dm;
	diagnosticMenuInit(&dm, &ctx);

	while (!dm.done) {
		diagnosticMenuLoop(&dm);
	}

	endwin();

	return 0;
}
