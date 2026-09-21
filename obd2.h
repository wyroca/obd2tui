#ifndef OBD2_H
#define OBD2_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#include "common.h"

#define OBD2_READER_ADDR 0xC0A8000A // 192.168.0.10
//#define OBD2_READER_ADDR 0x7f000001 // for testing
#define OBD2_READER_PORT 35000

typedef enum {
	UNDEFINED,
	CONNECTED,
	FAILED_TO_CONNECT // TODO - define specific failure states
} ConnectionState;

typedef struct {
	// TODO - optimize layout
	uint8_t service_mode;
	uint8_t *data; // variable length
} obd2_pid;

typedef struct {
	int sockfd;
	uint16_t fuel_system_status;
	uint8_t coolant_temp; // TODO - probably don't want this in here at the top level
	FILE *log_file;
	ConnectionState connection_state;
	pthread_cond_t connection_condition;
	pthread_mutex_t connection_state_mutex;
	bool is_valid;
	bool pids_supported[200];
} obd2_reader_ctx;

extern const char *obd2_pid_descriptions[];

void obd2_ctx_init(obd2_reader_ctx *ctx);
void *obd2_device_init(void *ctx_arg);
void obd2_reader_get_supported_pids_at(obd2_reader_ctx *ctx, const char *at);
void obd2_reader_get_all_supported_pids(obd2_reader_ctx *ctx);
void *obd2_receive_messages(void *ctx_arg);
void obd2_update_fuel_system_status(obd2_reader_ctx *ctx, char *recv_buf);
void obd2_update_coolant_temp(obd2_reader_ctx *ctx, char *recv_buf);


#endif // !OBD2_H
