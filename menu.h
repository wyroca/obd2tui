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

#endif // !MENU_H
