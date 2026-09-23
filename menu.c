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
		//werase(menu_window);
		switch (c) {
			case KEY_F(1):
				brk = true;
				break;
			case KEY_DOWN:
				if (curr_choice == idx - 2) {
					menu_driver(pid_menu, REQ_LAST_ITEM);
					break;
				}
				werase(info_window);
				curr_choice = min(curr_choice + 1, idx - 2);
				menu_driver(pid_menu, REQ_DOWN_ITEM);
				diagnosticMenuDisplayPidData(dm, info_window, curr_choice);
				break;
			case KEY_UP:
				if (curr_choice == 0) {
					menu_driver(pid_menu, REQ_FIRST_ITEM);
					break;
				}
				werase(info_window);
				curr_choice--;
				curr_choice = max(curr_choice, 0);
				menu_driver(pid_menu, REQ_UP_ITEM);
				diagnosticMenuDisplayPidData(dm, info_window, curr_choice);
				break;
			case 10:
				// TODO - handle enter
				break;
		}
		refresh();

		wrefresh(menu_window);

		wrefresh(info_window);

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

	if (!dm->ctx->pids[pid].supported) {
		mvwprintw(info_window, 0, 0, "NOT SUPPORTED");
		return;
	}

	switch (pid) {
		case 0x03:
			diagnosticMenuDisplayFuelSystemStatus(dm, info_window);
			break;
		case 0x04:
			diagnosticMenuDisplayEngineLoad(dm, info_window);
			break;
		case 0x05:
			diagnosticMenuDisplayCoolantTemp(dm, info_window);
			break;
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
			diagnosticMenuDisplayFuelTrim(dm, info_window, pid);
			break;
		case 0x0A:
			diagnosticMenuDisplayFuelPressure(dm, info_window);
			break;
		case 0x0B:
			diagnosticMenuDisplayMAP(dm, info_window);
			break;
		case 0x0C:
			diagnosticMenuDisplayEngineSpeed(dm, info_window);
			break;
		case 0x0D:
			diagnosticMenuDisplayVehicleSpeed(dm, info_window);
			break;
		case 0x0E:
			diagnosticMenuDisplayTimingAdvance(dm, info_window);
			break;
		case 0x0F:
			diagnosticMenuDisplayIntakeAirTemp(dm, info_window);
			break;
		case 0x10:
			diagnosticMenuDisplayMAFSensorRate(dm, info_window);
			break;
		case 0x11:
			diagnosticMenuDisplayThrottlePosition(dm, info_window);
			break;
		case 0x12:
			diagnosticMenuDisplayCommandedSecondaryAirStatus(dm, info_window);
			break;
		case 0x13:
			diagnosticMenuDisplayOxygenSensorsPresent(dm, info_window);
			break;
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
			diagnosticMenuDisplayOxygenSensorData(dm, info_window, pid);
			break;
		case 0x1C:
			diagnosticMenuDisplayOBDStandardsConformedTo(dm, info_window);
			break;
		default:
			mvwprintw(info_window, 0, 0, "SUPPORTED - NOT IMPLEMENTED");
			break;
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

void diagnosticMenuDisplayEngineLoad(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[0x04].data);
	mvwprintw(info_window, 0, 0, "Engine Load: %f%%", (float)A * 100.0f / 128.0f);
}

void diagnosticMenuDisplayFuelTrim(DiagnosticMenu *dm, WINDOW *info_window, int pid) {
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[0x04].data);
	float fuel_trim = (float)A / 1.28f - 100;
	switch (pid) {
		case 0x06:
			mvwprintw(info_window, 0, 0, "Short Term Fuel Trim Bank 1: %f%%", fuel_trim);
			break;
		case 0x07:
			mvwprintw(info_window, 0, 0, "Long Term Fuel Trim Bank 1: %f%%", fuel_trim);
			break;
		case 0x08:
			mvwprintw(info_window, 0, 0, "Short Term Fuel Trim Bank 2: %f%%", fuel_trim);
			break;
		case 0x09:
			mvwprintw(info_window, 0, 0, "Long Term Fuel Trim Bank 2: %f%%", fuel_trim);
			break;
		default:
			break;
	}
}

void diagnosticMenuDisplayFuelPressure(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[0x0A].data);
	uint16_t fuel_pressure = 3 * A;
	mvwprintw(info_window, 0, 0, "Fuel Pressure: %d	kPa", fuel_pressure);
}

// Intake Manifold Absolute Pressure
void diagnosticMenuDisplayMAP(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t MAP = hex_chars_to_u8(dm->ctx->pids[0x0B].data);
	mvwprintw(info_window, 0, 0, "Fuel Pressure: %d	kPa", MAP);
}

void diagnosticMenuDisplayEngineSpeed(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t AB = hex_chars_to_u16(dm->ctx->pids[0x0C].data);
	float engine_speed = AB / 4.0f;
	mvwprintw(info_window, 0, 0, "Engine Speed: %f rpm", engine_speed);
}

void diagnosticMenuDisplayVehicleSpeed(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t vehicle_speed = hex_chars_to_u8(dm->ctx->pids[0x0D].data);
	mvwprintw(info_window, 0, 0, "Vehicle Speed: %d km/h", vehicle_speed);
}

void diagnosticMenuDisplayTimingAdvance(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[0x0E].data);
	float timing_advance = (float)A / 2.0f - 64.0f;
	mvwprintw(info_window, 0, 0, "Timing Advance: %f degrees", timing_advance);
}

void diagnosticMenuDisplayIntakeAirTemp(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[0x0F].data);
	mvwprintw(info_window, 0, 0, "Intake Air Temp: %d C", A - 40);
}

void diagnosticMenuDisplayMAFSensorRate(DiagnosticMenu *dm, WINDOW *info_window) {
	uint16_t AB = hex_chars_to_u16(dm->ctx->pids[0x10].data);
	float air_flow_rate = (float)AB / 100.0f;
	mvwprintw(info_window, 0, 0, "MAF Sensor Air Flow Rate: %f g/s", air_flow_rate);
}

void diagnosticMenuDisplayThrottlePosition(DiagnosticMenu *dm, WINDOW *info_window) {
	uint16_t A = hex_chars_to_u8(dm->ctx->pids[0x11].data);
	float throttle_position = (float)A / 100.0f;
	mvwprintw(info_window, 0, 0, "Throttle Position: %f%%", throttle_position);
}

void diagnosticMenuDisplayCommandedSecondaryAirStatus(DiagnosticMenu *dm, WINDOW *info_window) {
	uint16_t A = hex_chars_to_u8(dm->ctx->pids[0x12].data);
	switch (A) {
		case 0x01:
			mvwprintw(info_window, 0, 0, "Upstream");
			break;
		case 0x02:
			mvwprintw(info_window, 0, 0, "Downstream of catalytic converter");
			break;
		case 0x04:
			mvwprintw(info_window, 0, 0, "From the outside atmosphere or off");
			break;
		case 0x08:
			mvwprintw(info_window, 0, 0, "Pump commanded on for diagnostics");
			break;
		default:
			mvwprintw(info_window, 0, 0, "Received invalid resopnse from device");
			break;
	}
}

void diagnosticMenuDisplayOxygenSensorsPresent(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[0x13].data);
	for (int i = 7; i >= 0; i--) {
		mvwprintw(info_window, i, 0, "Bank %d Sensor %d: %s", i > 3 ? 1 : 2, 4 - (i % 4), (A & (1 << i)) ? "Present" : "Not Present");
	}
}

void diagnosticMenuDisplayOxygenSensorData(DiagnosticMenu *dm, WINDOW *info_window, int pid) {
	int sensor_num = pid - 0x13;	
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[pid].data);
	uint8_t B = hex_chars_to_u8(dm->ctx->pids[pid].data + 1);
	if (B == 0xFF) {
		mvwprintw(info_window, 0, 0, "Sensor %d not used in trim calculation", sensor_num);
	} else {
		mvwprintw(info_window, 0, 0, "Sensor %d Voltage: %f V", sensor_num, (float)A / 200.0f);
		mvwprintw(info_window, 1, 0, "Sensor %dShort Term Fuel Trim: %f%%", sensor_num, 100.0f * (float)B / 128.0f - 100);
	}
}

void diagnosticMenuDisplayOBDStandardsConformedTo(DiagnosticMenu *dm, WINDOW *info_window) {
	uint8_t A = hex_chars_to_u8(dm->ctx->pids[0x1C].data);
	switch (A) {
		case 1:
			mvwprintw(info_window, 0, 0, "OBD-II as defined by the CARB");
			break;
		case 2:
			mvwprintw(info_window, 0, 0, "OBD as defined by the EPA");
			break;
		case 3:
			mvwprintw(info_window, 0, 0, "OBD and OBD-II");
			break;
		case 4:
			mvwprintw(info_window, 0, 0, "OBD-I");
			break;
		case 5:
			mvwprintw(info_window, 0, 0, "Not OBD compliant");
			break;
		case 6:
			mvwprintw(info_window, 0, 0, "EOBD (Europe)");
			break;
		case 7:
			mvwprintw(info_window, 0, 0, "EOBD and OBD-II");
			break;
		case 8:
			mvwprintw(info_window, 0, 0, "EOBD and OBD");
			break;
		case 9:
			mvwprintw(info_window, 0, 0, "EOBD, OBD and OBD II");
			break;
		case 10:
			mvwprintw(info_window, 0, 0, "JOBD (Japan)");
			break;
		case 11:
			mvwprintw(info_window, 0, 0, "JOBD and OBD II");
			break;
		case 12:
			mvwprintw(info_window, 0, 0, "JOBD and EOBD");
			break;
		case 13:
			mvwprintw(info_window, 0, 0, "JOBD, EOBD, and OBD II");
			break;
		case 14:
			mvwprintw(info_window, 0, 0, "OBD, EOBD, and KOBD");
			break;
		case 15:
			mvwprintw(info_window, 0, 0, "OBD, OBD II, EOBD, and KOBD");
			break;
		case 16:
			mvwprintw(info_window, 0, 0, "Reserved");
			break;
		case 17:
			mvwprintw(info_window, 0, 0, "Engine Manufacturer Diagnostics (EMD)");
			break;
		case 18:
			mvwprintw(info_window, 0, 0, "Engine Manufacturer Diagnostics Enhanced (EMD+)");
			break;
		case 19:
			mvwprintw(info_window, 0, 0, "Heavy Duty On-Board Diagnostics (Child/Partial) (HD OBD-C)");
			break;
		case 20:
			mvwprintw(info_window, 0, 0, "Heavy Duty On-Board Diagnostics (HD OBD)");
			break;
		case 21:
			mvwprintw(info_window, 0, 0, "World Wide Harmonized OBD (WWH OBD)");
			break;
		case 22:
			mvwprintw(info_window, 0, 0, "Reserved");
			break;
		case 23:
			mvwprintw(info_window, 0, 0, "Heavy Duty Euro OBD Stage I without NOx control (HD EOBD-I)");
			break;
		case 24:
			mvwprintw(info_window, 0, 0, "Heavy Duty Euro OBD Stage I with NOx control (HD EOBD-I N)");
			break;
		case 25:
			mvwprintw(info_window, 0, 0, "Heavy Duty Euro OBD Stage II without NOx control (HD EOBD-II)");
			break;
		case 26:
			mvwprintw(info_window, 0, 0, "Heavy Duty Euro OBD Stage II with NOx control (HD EOBD-II N)");
			break;
		case 27:
			mvwprintw(info_window, 0, 0, "Heavy Duty ZEV");
			break;
		case 28:
			mvwprintw(info_window, 0, 0, "Brazil OBD Phase 1 (OBDBr-1)");
			break;
		case 29:
			mvwprintw(info_window, 0, 0, "Brazil OBD Phase 2 (OBDBr-2)");
			break;
		case 30:
			mvwprintw(info_window, 0, 0, "Korean OBD (KOBD)");
			break;
		case 31:
			mvwprintw(info_window, 0, 0, "India OBD I (IOBD I)");
			break;
		case 32:
			mvwprintw(info_window, 0, 0, "India OBD II (IOBD II)");
			break;
		case 33:
			mvwprintw(info_window, 0, 0, "Heavy Duty Euro OBD Stage VI (HD EOBD-IV)");
			break;
		case 34:
			mvwprintw(info_window, 0, 0, "OBD, OBD-II, and HD OBD");
			break;
		case 35:
			mvwprintw(info_window, 0, 0, "Brazil OBD Phase 3 (OBDBr-3)");
			break;
		case 251:
		case 252:
		case 253:
		case 254:
		case 255:
			mvwprintw(info_window, 0, 0, "Not available for assignment (SAE J1939 special meaning)");
			break;
		default:
			mvwprintw(info_window, 0, 0, "Reserved");
			break;
	}
}
