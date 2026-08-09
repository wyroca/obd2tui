#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// #define OBD2_READER_ADDR 0xC0A8000A // 192.168.0.10
#define OBD2_READER_ADDR 0x7f000001 // for testing
#define OBD2_READER_PORT 35000

void die(const char *error_message) {
	printf("%s\n", error_message);
	exit(1);
}

int main(int argc, char *argv[]) {
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

	return 0;
}
