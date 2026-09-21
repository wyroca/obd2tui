#ifndef MENU_H
#define MENU_H

#include "obd2.h"

#include <menu.h>
#include <stdbool.h>
#include <sys/cdefs.h>

typedef struct {
	int curr_choice;
	MENU *menu;
	obd2_reader_ctx *ctx;
	bool done;
} DiagnosticMenu;

void diagnosticMenuInit(DiagnosticMenu *dm, obd2_reader_ctx *ctx);
void diagnosticMenuLoop(DiagnosticMenu *dm);
void diagnosticMenuDisplayVehicleInfo(DiagnosticMenu *dm __attribute_maybe_unused__);
void diagnosticMenuDisplayPidData(DiagnosticMenu *dm __attribute_maybe_unused__);
void diagnosticMenuGetPidData(DiagnosticMenu *dm __attribute_maybe_unused__, WINDOW *info_window, int pid);

#endif // !MENU_H
