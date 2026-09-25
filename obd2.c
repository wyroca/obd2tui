#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/tcp.h>

#include "arena.h"
#include "common.h"
#include "obd2.h"

const char* obd2_pid_descriptions[] = {
	[0x00] = "PIDs supported [$01 - $20]",
	[0x01] = "Monitor status since DTCs cleared.",
	[0x02] = "DTC that caused freeze frame to be stored.",
	[0x03] = "Fuel system status",
	[0x04] = "Calculated engine load",
	[0x05] = "Engine coolant temperature",
	[0x06] = "Short term fuel trim (STFT) - Bank 1",
	[0x07] = "Long term fuel trim (LTFT) - Bank 1",
	[0x08] = "Short term fuel trim (STFT) - Bank 2",
	[0x09] = "Long term fuel trim (LTFT) - Bank 2",
	[0x0A] = "Fuel pressure (gauge pressure)",
	[0x0B] = "Intake manifold absolute pressure",
	[0x0C] = "Engine speed",
	[0x0D] = "Vehicle speed",
	[0x0E] = "Timing advance",
	[0x0F] = "Intake air temperature",
	[0x10] = "Mass air flow sensor (MAF) air flow rate",
	[0x11] = "Throttle position",
	[0x12] = "Commanded secondary air status",
	[0x13] = "Oxygen sensors present (in 2 banks)",
	[0x14] = "Oxygen Sensor 1 Voltage, STFT",
	[0x15] = "Oxygen Sensor 2 Voltage, STFT",
	[0x16] = "Oxygen Sensor 3",
	[0x17] = "Oxygen Sensor 4",
	[0x18] = "Oxygen Sensor 5",
	[0x19] = "Oxygen Sensor 6",
	[0x1A] = "Oxygen Sensor 7",
	[0x1B] = "Oxygen Sensor 8",
	[0x1C] = "OBD standards this vehicle conforms to",
	[0x1D] = "Oxygen sensors present (in 4 banks)",
	[0x1E] = "Auxiliary input status",
	[0x1F] = "Run time since engine start",
	[0x20] = "PIDs supported [$21 - $40]",
	[0x21] = "Distance traveled with malfunction indicator lamp (MIL) on",
	[0x22] = "Fuel Rail Pressure (relative to manifold vacuum)",
	[0x23] = "Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)",
	[0x24] = "Oxygen Sensor 1 Air-Fuel Equivalence Ratio, Voltage",
	[0x25] = "Oxygen Sensor 2 Air-Fuel Equivalence Ratio, Voltage",
	[0x26] = "Oxygen Sensor 3 Air-Fuel Equivalence Ratio, Voltage",
	[0x27] = "Oxygen Sensor 4 Air-Fuel Equivalence Ratio, Voltage",
	[0x28] = "Oxygen Sensor 5 Air-Fuel Equivalence Ratio, Voltage",
	[0x29] = "Oxygen Sensor 6 Air-Fuel Equivalence Ratio, Voltage",
	[0x2A] = "Oxygen Sensor 7 Air-Fuel Equivalence Ratio, Voltage",
	[0x2B] = "Oxygen Sensor 8 Air-Fuel Equivalence Ratio, Voltage",
	[0x2C] = "Commanded EGR",
	[0x2D] = "EGR Error",
	[0x2E] = "Commanded evaporative purge",
	[0x2F] = "Fuel Tank Level Input",
	[0x30] = "Warm-ups since codes cleared",
	[0x31] = "Distance traveled since codes cleared",
	[0x32] = "Evap. System Vapor Pressure",
	[0x33] = "Absolute Barometric Pressure",
	[0x34] = "Oxygen Sensor 1 Air-Fuel Equivalence Ratio, Current",
	[0x35] = "Oxygen Sensor 2 Air-Fuel Equivalence Ratio, Current",
	[0x36] = "Oxygen Sensor 3 Air-Fuel Equivalence Ratio, Current",
	[0x37] = "Oxygen Sensor 4 Air-Fuel Equivalence Ratio, Current",
	[0x38] = "Oxygen Sensor 5 Air-Fuel Equivalence Ratio, Current",
	[0x39] = "Oxygen Sensor 6 Air-Fuel Equivalence Ratio, Current",
	[0x3A] = "Oxygen Sensor 7 Air-Fuel Equivalence Ratio, Current",
	[0x3B] = "Oxygen Sensor 8 Air-Fuel Equivalence Ratio, Current",
	[0x3C] = "Catalyst Temperature: Bank 1, Sensor 1",
	[0x3D] = "Catalyst Temperature: Bank 2, Sensor 1",
	[0x3E] = "Catalyst Temperature: Bank 1, Sensor 2",
	[0x3F] = "Catalyst Temperature: Bank 2, Sensor 2",
	[0x40] = "PIDs supported [$41 - $60]",
	[0x41] = "Monitor status this drive cycle",
	[0x42] = "Control module voltage",
	[0x43] = "Absolute load value",
	[0x44] = "Commanded Air-Fuel Equivalence Ratio",
	[0x45] = "Relative throttle position",
	[0x46] = "Ambient air temperature",
	[0x47] = "Absolute throttle position B",
	[0x48] = "Absolute throttle position C",
	[0x49] = "Accelerator pedal position D",
	[0x4A] = "Accelerator pedal position E",
	[0x4B] = "Accelerator pedal position F",
	[0x4C] = "Commanded throttle actuator",
	[0x4D] = "Time run with MIL on",
	[0x4E] = "Time since trouble codes cleared",
	[0x4F] = "Maximum value for Fuel–Air equivalence ratio, oxygen sensor voltage, oxygen sensor current, and intake manifold absolute pressure",
	[0x50] = "Maximum value for air flow rate from mass air flow sensor",
	[0x51] = "Fuel Type",
	[0x52] = "Ethanol fuel",
	[0x53] = "Absolute Evap system Vapor Pressure",
	[0x54] = "Evap system vapor pressure",
	[0x55] = "Short term secondary oxygen sensor trim, banks 1 and 3",
	[0x56] = "Long term secondary oxygen sensor trim, banks 1 and 3",
	[0x57] = "Short term secondary oxygen sensor trim, banks 2 and 4",
	[0x58] = "Long term secondary oxygen sensor trim, banks 2 and 4",
	[0x59] = "Fuel rail absolute pressure",
	[0x5A] = "Relative accelerator pedal position",
	[0x5B] = "Hybrid battery pack remaining life",
	[0x5C] = "Engine oil temperature",
	[0x5D] = "Fuel injection timing",
	[0x5E] = "Engine fuel rate",
	[0x5F] = "Emission requirements to which vehicle is designed",
	[0x60] = "PIDs supported [$61 - $80]",
	[0x61] = "Driver's demand engine - percent torque",
	[0x62] = "Actual engine - percent torque",
	[0x63] = "Engine reference torque",
	[0x64] = "Engine percent torque data",
	[0x65] = "Auxiliary input / output supported",
	[0x66] = "Mass air flow sensor",
	[0x67] = "Engine coolant temperature",
	[0x68] = "Intake air temperature sensor",
	[0x69] = "Actual EGR, Commanded EGR, and EGR Error",
	[0x6A] = "Commanded Diesel intake air flow control and relative intake air flow position",
	[0x6B] = "Exhaust gas recirculation temperature",
	[0x6C] = "Commanded throttle actuator control and relative throttle position",
	[0x6D] = "Fuel pressure control system",
	[0x6E] = "Injection pressure control system",
	[0x6F] = "Turbocharger compressor inlet pressure",
	[0x70] = "Boost pressure control",
	[0x71] = "Variable Geometry turbo (VGT) control",
	[0x72] = "Wastegate control",
	[0x73] = "Exhaust pressure",
	[0x74] = "Turbocharger RPM",
	[0x75] = "Turbocharger temperature",
	[0x76] = "Turbocharger temperature",
	[0x77] = "Charge air cooler temperature (CACT)",
	[0x78] = "Exhaust Gas temperature (EGT) Bank 1",
	[0x79] = "Exhaust Gas temperature (EGT) Bank",
	[0x7A] = "Diesel particulate filter (DPF) differential pressure",
	[0x7B] = "Diesel particulate filter (DPF)",
	[0x7C] = "Diesel Particulate filter (DPF) temperature",
	[0x7D] = "NOx NTE (Not-To-Exceed) control area status",
	[0x7E] = "PM NTE (Not-To-Exceed) control area status",
	[0x7F] = "Engine run time",
	[0x80] = "PIDs supported [$81 - $A0]",
	[0x81] = "Engine run time for Auxiliary Emissions Control Device(AECD)",
	[0x82] = "Engine run time for Auxiliary Emissions Control Device(AECD)",
	[0x83] = "NOx sensor",
	[0x84] = "Manifold surface temperature",
	[0x85] = "NOx reagent system",
	[0x86] = "Particulate matter (PM) sensor",
	[0x87] = "Intake manifold absolute pressure",
	[0x88] = "SCR Induce System",
	[0x89] = "Run Time for AECD #11-#15",
	[0x8A] = "Run Time for AECD #16-#20",
	[0x8B] = "Diesel Aftertreatment",
	[0x8C] = "O2 Sensor (Wide Range)",
	[0x8D] = "Throttle Position G",
	[0x8E] = "Engine Friction - Percent Torque",
	[0x8F] = "PM Sensor Bank 1 & 2",
	[0x90] = "WWH-OBD Vehicle OBD System Information",
	[0x91] = "WWH-OBD Vehicle OBD System Information",
	[0x92] = "Fuel System Control",
	[0x93] = "WWH-OBD Vehicle OBD Counters support",
	[0x94] = "NOx Warning And Inducement System",
	[0x98] = "Exhaust Gas Temperature Sensor",
	[0x99] = "Exhaust Gas Temperature Sensor",
	[0x9A] = "Hybrid/EV Vehicle System Data, Battery, Voltage",
	[0x9B] = "Diesel Exhaust Fluid Sensor Data",
	[0x9C] = "O2 Sensor Data",
	[0x9D] = "Engine Fuel Rate",
	[0x9E] = "Engine Exhaust Flow Rate",
	[0x9F] = "Fuel System Percentage Use",
	[0xA0] = "PIDs supported [$A1 - $C0]",
	[0xA1] = "NOx Sensor Corrected Data",
	[0xA2] = "Cylinder Fuel Rate",
	[0xA3] = "Evap System Vapor Pressure",
	[0xA4] = "Transmission Actual Gear",
	[0xA5] = "Commanded Diesel Exhaust Fluid Dosing",
	[0xA6] = "Odometer",
	[0xA7] = "NOx Sensor Concentration Sensors 3 and 4",
	[0xA8] = "NOx Sensor Corrected Concentration Sensors 3 and 4",
	[0xA9] = "ABS Disable Switch State",
	[0xC0] = "PIDs supported [$C1 - $E0]",
	[0xC3] = "Fuel Level Input A/B",
	[0xC4] = "Exhaust Particulate Control System Diagnostic Time/Count",
	[0xC5] = "Fuel Pressure A and B",
	[0xC6] = "Byte 1 - Particulate control - driver inducement system status; Byte 2,3 - Removal or block of the particulate aftertreatment system counter; Byte 4,5 - Liquid regent injection system (e.g. fuel-borne catalyst) failure counter; Byte 6,7 - Malfunction of Particulate control monitoring system counter",
	[0xC7] = "Distance Since Reflash or Module Replacement",
	[0xC8] = "NOx Control Diagnostic (NCD) and Particulate Control Diagnostic (PCD) Warning Lamp status"
};

const size_t obd2_pid_data_sizes[] = {
	[0x00] = 4,
	[0x01] = 4,
	[0x02] = 2,
	[0x03] = 2,
	[0x04] = 1,
	[0x05] = 1,
	[0x06] = 1,
	[0x07] = 1,
	[0x08] = 1,
	[0x09] = 1,
	[0x0A] = 1,
	[0x0B] = 1,
	[0x0C] = 2,
	[0x0D] = 1,
	[0x0E] = 1,
	[0x0F] = 1,
	[0x10] = 2,
	[0x11] = 1,
	[0x12] = 1,
	[0x13] = 1,
	[0x14] = 2,
	[0x15] = 2,
	[0x16] = 2,
	[0x17] = 2,
	[0x18] = 2,
	[0x19] = 2,
	[0x1A] = 2,
	[0x1B] = 2,
	[0x1C] = 1,
	[0x1D] = 1,
	[0x1E] = 1,
	[0x1F] = 2,
	[0x20] = 4,
	[0x21] = 2,
	[0x22] = 2,
	[0x23] = 2,
	[0x24] = 4,
	[0x25] = 4,
	[0x26] = 4,
	[0x27] = 4,
	[0x28] = 4,
	[0x29] = 4,
	[0x2A] = 4,
	[0x2B] = 4,
	[0x2C] = 1,
	[0x2D] = 1,
	[0x2E] = 1,
	[0x2F] = 1,
	[0x30] = 1,
	[0x31] = 2,
	[0x32] = 2,
	[0x33] = 1,
	[0x34] = 4,
	[0x35] = 4,
	[0x36] = 4,
	[0x37] = 4,
	[0x38] = 4,
	[0x39] = 4,
	[0x3A] = 4,
	[0x3B] = 4,
	[0x3C] = 2,
	[0x3D] = 2,
	[0x3E] = 2,
	[0x3F] = 2,
	[0x40] = 4,
	[0x41] = 4,
	[0x42] = 2,
	[0x43] = 2,
	[0x44] = 2,
	[0x45] = 1,
	[0x46] = 1,
	[0x47] = 1,
	[0x48] = 1,
	[0x49] = 1,
	[0x4A] = 1,
	[0x4B] = 1,
	[0x4C] = 1,
	[0x4D] = 2,
	[0x4E] = 2,
	[0x4F] = 4,
	[0x50] = 4,
	[0x51] = 1,
	[0x52] = 1,
	[0x53] = 2,
	[0x54] = 2,
	[0x55] = 2,
	[0x56] = 2,
	[0x57] = 2,
	[0x58] = 2,
	[0x59] = 2,
	[0x5A] = 1,
	[0x5B] = 1,
	[0x5C] = 1,
	[0x5D] = 2,
	[0x5E] = 2,
	[0x5F] = 1,
	[0x60] = 4,
	[0x61] = 1,
	[0x62] = 1,
	[0x63] = 2,
	[0x64] = 5,
	[0x65] = 2,
	[0x66] = 5,
	[0x67] = 3,
	[0x68] = 3,
	[0x69] = 7,
	[0x6A] = 5,
	[0x6B] = 5,
	[0x6C] = 5,
	[0x6D] = 11,
	[0x6E] = 9,
	[0x6F] = 3,
	[0x70] = 10,
	[0x71] = 6,
	[0x72] = 5,
	[0x73] = 5,
	[0x74] = 5,
	[0x75] = 7,
	[0x76] = 7,
	[0x77] = 5,
	[0x78] = 9,
	[0x79] = 9,
	[0x7A] = 7,
	[0x7B] = 7,
	[0x7C] = 9,
	[0x7D] = 1,
	[0x7E] = 1,
	[0x7F] = 13,
	[0x80] = 4,
	[0x81] = 41,
	[0x82] = 41,
	[0x83] = 9,
	[0x84] = 1,
	[0x85] = 10,
	[0x86] = 5,
	[0x87] = 5,
	[0x88] = 13,
	[0x89] = 41,
	[0x8A] = 41,
	[0x8B] = 7,
	[0x8C] = 17,
	[0x8D] = 1,
	[0x8E] = 1,
	[0x8F] = 7,
	[0x90] = 3,
	[0x91] = 5,
	[0x92] = 2,
	[0x93] = 3,
	[0x94] = 12,
	[0x98] = 9,
	[0x99] = 9,
	[0x9A] = 6,
	[0x9B] = 4,
	[0x9C] = 17,
	[0x9D] = 4,
	[0x9E] = 2,
	[0x9F] = 9,
	[0xA0] = 4,
	[0xA1] = 9,
	[0xA2] = 2,
	[0xA3] = 9,
	[0xA4] = 4,
	[0xA5] = 4,
	[0xA6] = 4,
	[0xA7] = 4,
	[0xA8] = 4,
	[0xA9] = 4,
	[0xC0] = 4,
	[0xC3] = 2,
	[0xC4] = 8,
	[0xC5] = 4,
	[0xC6] = 7,
	[0xC7] = 2,
	[0xC8] = 1
};

void obd2_ctx_init(obd2_reader_ctx *ctx, arena *a) {
	ctx->is_valid = false;
	pthread_mutex_init(&ctx->connection_state_mutex, NULL);
	ctx->connection_state = UNDEFINED;
	
	// TODO - gotta be more robust eventually
	ctx->log_file = fopen("log.txt", "w");

	ctx->mem_arena = a;
	ctx->num_pids = 200;
	ctx->pids = ARENA_ALLOC(a, obd2_pid, ctx->num_pids);

	for (size_t i = 0; i < ctx->num_pids; i++) {
		ctx->pids[i].data = ARENA_ALLOC(a, obd2_pid, obd2_pid_data_sizes[i] * 2);
		ctx->pids[i].service_mode = 1; // TODO - will need to break out into separate service modes once they are supported
		ctx->pids[i].supported = false;
	}
}

void *obd2_device_init(void *ctx_arg) {
	obd2_reader_ctx *ctx = (obd2_reader_ctx*)ctx_arg;
	ctx->is_valid = false;

	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		//die("Failed to create client TCP socket\n");
	}

	int optval = 1;
	socklen_t len = sizeof(optval);
	int rc = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, len);
	if (rc == -1) {
		fprintf(ctx->log_file, "SETSOCKOPT FAILED\n");
	}
	// TODO - error handling

	struct sockaddr_in obd2_reader_addr = {0};
	socklen_t obd2_reader_addr_len = sizeof(obd2_reader_addr);
	obd2_reader_addr.sin_addr.s_addr = htonl(OBD2_READER_ADDR);
	obd2_reader_addr.sin_port = htons(OBD2_READER_PORT);
	obd2_reader_addr.sin_family = AF_INET;

	rc = connect(sockfd, (struct sockaddr *)&obd2_reader_addr, obd2_reader_addr_len);
	pthread_mutex_lock(&ctx->connection_state_mutex);
	if (rc == -1) {
		ctx->connection_state = FAILED_TO_CONNECT;
	} else {
		ctx->connection_state = CONNECTED;
		ctx->is_valid = true;
	}
	pthread_mutex_unlock(&ctx->connection_state_mutex);

	pthread_cond_broadcast(&ctx->connection_condition);

	ctx->sockfd = sockfd;

	return NULL;
}

void obd2_reader_get_supported_pids_at(obd2_reader_ctx *ctx, const char *at) {
	char send_buf[6] = {0};
	strncpy(send_buf, "01 ", 4);
	send_buf[3] = at[0];
	send_buf[4] = at[1];
	send_buf[5] = '\r';
	send(ctx->sockfd, send_buf, 6, 0); // 01 = Service 1 (show current data), 00 = PID 0 (ask for support for PIDs 1 - 32)
}

void obd2_reader_get_all_supported_pids(obd2_reader_ctx *ctx) {
	/*
	 * TODO - even with TCP_NODELAY, sending successive
	 * messages results in coalesced packets, which doesn't seem
	 * to work with ELM327
	*/
	obd2_reader_get_supported_pids_at(ctx, "00");
	usleep(240 * 1e3);
	obd2_reader_get_supported_pids_at(ctx, "20");
	usleep(240 * 1e3);
	obd2_reader_get_supported_pids_at(ctx, "40");
	usleep(240 * 1e3);
	obd2_reader_get_supported_pids_at(ctx, "60");
	usleep(240 * 1e3);
	obd2_reader_get_supported_pids_at(ctx, "80");
	usleep(240 * 1e3);
	obd2_reader_get_supported_pids_at(ctx, "A0");
	usleep(240 * 1e3);
	obd2_reader_get_supported_pids_at(ctx, "C0");
	usleep(240 * 1e3);

	// Get coolant temp
	// TODO - do NOT keep this here and DO make this its own function
	char send_buf[] = "01 05\r";
	send(ctx->sockfd, send_buf, 6, 0); // 01 = Service 1 (show current data), 00 = PID 0 (ask for support for PIDs 1 - 32)
	
}

void *obd2_receive_messages(void *ctx_arg) {
	obd2_reader_ctx *ctx = (obd2_reader_ctx*)ctx_arg;

	while (1) {
		char recv_buf[64] = {0}; // TODO - 64 proper size?
		ssize_t num_bytes = recv(ctx->sockfd, recv_buf, 63, 0);
		if (num_bytes == -1) {
			fprintf(ctx->log_file, "Error reading message from OBD2 device\n");
			fflush(ctx->log_file);
		}
		// TODO - special handling of "NO DATA" response?

		if (strncmp(recv_buf, "41 ", 3) == 0) {
			uint8_t idx = hex_chars_to_u8(recv_buf + 3);
			uint32_t supported_bits;

			switch (idx) {
				case 0x00:
				case 0x20:
				case 0x40:
				case 0x60:
				case 0x80:
				case 0xA0:
				case 0xC0:
					supported_bits = hex_chars_to_u32(recv_buf + 6);
					for (int i = idx; i < idx + 32; i++) {
						ctx->pids[i].supported = false;
						if ((1 << (31 - (i - idx))) & supported_bits) {
							ctx->pids[i].supported = true;
						}
					}
					break;
				case 0x03: // Fuel System Status
				case 0x05: // Coolant Temp
				case 0x0C:
					obd2_update_pid_data(ctx, recv_buf, idx);
					break;
				case 0x06: // STFT Bank 1
					break;
			}
		}
	}

	return NULL;
}

void *obd2_send_requests(void *ctx_arg) {
	usleep(800 * 1e3);
	obd2_reader_ctx *ctx = (obd2_reader_ctx*)ctx_arg;

	char send_buf[6] = {0};
	while (1) {
		usleep(1000 * 1e3); // TODO - tune this parameter

		send_buf[0] = '0';
		send_buf[1] = '1';
		send_buf[2] = ' ';

		u8_to_hex_chars(send_buf + 3, ctx->pid_requesting);
		send_buf[5] = '\r';
		send(ctx->sockfd, send_buf, 6, 0);

		memset(send_buf, 0, 6);	
	}

	return NULL;
}

void obd2_update_pid_data(obd2_reader_ctx *ctx, char *recv_buf, int pid) {
	memcpy(ctx->pids[pid].data, recv_buf + 6, obd2_pid_data_sizes[pid] * 2);
}
