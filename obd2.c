#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#include "obd2.h"

void obd2_reader_init(obd2_reader_ctx *ctx) {
	ctx->is_valid = false;

	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		die("Failed to create client TCP socket\n");
	}

	struct sockaddr_in obd2_reader_addr = {0};
	socklen_t obd2_reader_addr_len = sizeof(obd2_reader_addr);
	obd2_reader_addr.sin_addr.s_addr = htonl(OBD2_READER_ADDR);
	obd2_reader_addr.sin_port = htons(OBD2_READER_PORT);
	obd2_reader_addr.sin_family = AF_INET;

	int rc = connect(sockfd, (struct sockaddr *)&obd2_reader_addr, obd2_reader_addr_len);
	if (rc == -1) {
		die("Failed to connect to OBD2 reader\n");
	}

	ctx->sockfd = sockfd;
}

void obd2_reader_get_supported_pids_at(obd2_reader_ctx *ctx, const char *at) {
	char recv_buf[64] = {0};
	char send_buf[6] = {0};
	strncpy(send_buf, "01 ", 4);
	send_buf[3] = at[0];
	send_buf[4] = at[1];
	send_buf[5] = '\r';
	send(ctx->sockfd, send_buf, 6, 0); // 01 = Service 1 (show current data), 00 = PID 0 (ask for support for PIDs 1 - 32)
	recv(ctx->sockfd, recv_buf, 64, 0); // reader echos back, need to receive this
	memset(recv_buf, 0, sizeof(recv_buf));
	recv(ctx->sockfd, recv_buf, 64, 0);
	if (strncmp(recv_buf, "NO DATA", 7) == 0) {
		return;
	}
	if (strncmp(recv_buf, "41", 2) != 0 && strncmp(recv_buf + 3, at, 2) != 0) {
		printf("Did not receive expected response 41 %s confirming request for PID 0\n", at);
	}

	long offset = strtol(at, NULL, 16);
	long supported_pids = strtol(recv_buf + 10, NULL, 16);
	for (int i = 0; i < 32; i++) {
		ctx->pids_supported[i + offset] = false;
		if ((1 << (31 - i)) & supported_pids) {
			ctx->pids_supported[i + offset] = true;
		}
	}
}

void obd2_reader_get_all_supported_pids(obd2_reader_ctx *ctx) {
	obd2_reader_get_supported_pids_at(ctx, "00");
	obd2_reader_get_supported_pids_at(ctx, "20");
	obd2_reader_get_supported_pids_at(ctx, "40");
	obd2_reader_get_supported_pids_at(ctx, "60");
	obd2_reader_get_supported_pids_at(ctx, "80");
	obd2_reader_get_supported_pids_at(ctx, "A0");
	obd2_reader_get_supported_pids_at(ctx, "C0");
}
