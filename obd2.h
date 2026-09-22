#ifndef OBD2_H
#define OBD2_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#include "arena.h"
#include "common.h"

//#define OBD2_READER_ADDR 0xC0A8000A // 192.168.0.10
#define OBD2_READER_ADDR 0x7f000001 // for testing
#define OBD2_READER_PORT 35000

typedef enum {
	UNDEFINED,
	CONNECTED,
	FAILED_TO_CONNECT // TODO - define specific failure states
} ConnectionState;

typedef struct {
	// TODO - optimize layout
	char *data; // variable length
	uint8_t service_mode;
	bool supported;
} obd2_pid;

typedef struct {
	arena *mem_arena;
	int sockfd;
	FILE *log_file;
	ConnectionState connection_state;
	pthread_cond_t connection_condition;
	pthread_mutex_t connection_state_mutex;
	bool is_valid;
	obd2_pid *pids;
	size_t num_pids;
} obd2_reader_ctx;

extern const char *obd2_pid_descriptions[];
extern const size_t obd2_pid_data_sizes[];

void obd2_ctx_init(obd2_reader_ctx *ctx, arena *a);
void *obd2_device_init(void *ctx_arg);
void obd2_reader_get_supported_pids_at(obd2_reader_ctx *ctx, const char *at);
void obd2_reader_get_all_supported_pids(obd2_reader_ctx *ctx);
void *obd2_receive_messages(void *ctx_arg);
void obd2_update_pid_data(obd2_reader_ctx *ctx, char *recv_buf, int pid);


#endif // !OBD2_H
