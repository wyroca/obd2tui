#include <netinet/in.h>
#include <pthread.h>
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
	initscr();	
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	refresh();

	obd2_reader_ctx ctx;
	arena *a = arena_create(10000); // how much do we need?
	obd2_ctx_init(&ctx, a);


	void *res;
	pthread_t device_connect_thread, receiver_thread;

	pthread_create(&device_connect_thread, NULL, obd2_device_init, &ctx);

	mvprintw(0, 0, "CONNECTING...");
	refresh();
	while (ctx.connection_state == UNDEFINED) {
		pthread_cond_wait(&ctx.connection_condition, &ctx.connection_state_mutex);
	}

	pthread_join(device_connect_thread, &res);

	pthread_create(&receiver_thread, NULL, obd2_receive_messages, &ctx);

	obd2_reader_get_all_supported_pids(&ctx);

	wclear(stdscr);
	mvprintw(0, 0, "DONE???");
	getch();
	refresh();

	DiagnosticMenu dm;
	diagnosticMenuInit(&dm, &ctx);

	while (!dm.done) {
		diagnosticMenuLoop(&dm);
	}

	pthread_join(receiver_thread, &res);

	endwin();

	return 0;
}
