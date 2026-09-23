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
void diagnosticMenuDisplayPidMenu(DiagnosticMenu *dm __attribute_maybe_unused__);
void diagnosticMenuDisplayPidData(DiagnosticMenu *dm __attribute_maybe_unused__, WINDOW *info_window, int pid);
void diagnosticMenuDisplayCoolantTemp(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayFuelSystemStatus(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayEngineLoad(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayFuelTrim(DiagnosticMenu *dm, WINDOW *info_window, int pid);
void diagnosticMenuDisplayFuelPressure(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayMAP(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayEngineSpeed(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayVehicleSpeed(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayTimingAdvance(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayIntakeAirTemp(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayMAFSensorRate(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayThrottlePosition(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayCommandedSecondaryAirStatus(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayOxygenSensorsPresent(DiagnosticMenu *dm, WINDOW *info_window);
void diagnosticMenuDisplayOxygenSensorData(DiagnosticMenu *dm, WINDOW *info_window, int pid);
void diagnosticMenuDisplayOBDStandardsConformedTo(DiagnosticMenu *dm, WINDOW *info_window);

#endif // !MENU_H
