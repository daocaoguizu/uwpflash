#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include "interface.h"
#include "command.h"

static sem_t cmd_sem;

int send_command(int cmd, int timeout)
{
	int ret;
	struct packet_header pkt_hdr;

	pkt_hdr.type = le16(cmd);
	pkt_hdr.size = 0;

	ret = intf_send((unsigned char *)&pkt_hdr,
			sizeof(pkt_hdr), 1, timeout);
	if (ret < 0) return ret;

	return cmd_wait(timeout);
}

int send_data(int type, unsigned char *data,
		int len, int timeout)
{
	int i;
	int ret;
	struct packet_header *pkt_hdr;
	unsigned char *buf;
	int buf_len = len + sizeof(struct packet_header);
	unsigned int *src, *dest;

	buf = malloc(buf_len);
	if (buf == NULL) {
		printf("out of memory.\n");
		return -1;
	}

	pkt_hdr = (struct packet_header *)buf;
	pkt_hdr->type = le16(type);
	pkt_hdr->size = le16(len);

	src = (unsigned int *)data;
	dest = (unsigned int *)(buf + sizeof(struct packet_header));

	memcpy((void *)dest, (void *)src, len); 

	ret = intf_send(buf, buf_len, 1, timeout);
	if (ret) {
		free(buf);
		return -1;
	}

	free(buf);
	return cmd_wait(timeout);
}

struct packet_start
{
	unsigned int addr;
	unsigned int len;
};

int cmd_send_start(unsigned int addr, unsigned int len)
{
	struct packet_start pkt_start;

	pkt_start.addr = le32(addr);
	pkt_start.len = le32(len);

	return send_data(BSL_CMD_START_DATA, (unsigned char *)&pkt_start,
			sizeof(pkt_start), 1000);
}

int cmd_send_data(unsigned char * addr, unsigned int len)
{
	return send_data(BSL_CMD_MIDST_DATA, addr, len, 1000);
}

int cmd_connect(int timeout)
{
	return send_command(BSL_CMD_CONNECT, timeout);
}

int cmd_stop(int timeout)
{
	return send_command(BSL_CMD_END_DATA, timeout);
}

int cmd_exec(int timeout)
{
	return send_command(BSL_CMD_EXEC_DATA, timeout);
}


int cmd_check_bandrate(int timeout)
{
	int ret;
	unsigned char check_val = BSL_CMD_CHECK_BAUD;

	ret = intf_send(&check_val, sizeof(check_val),
			0, timeout);
	if (ret < 0) return ret;

	return cmd_wait(timeout);
}

int cmd_wait(int timeout)
{
	int ret;
	struct timespec wait_time;

	clock_gettime(CLOCK_REALTIME, &wait_time);
	wait_time.tv_sec += timeout;
	ret = sem_timedwait(&cmd_sem, &wait_time);
	if ((ret == -1) && (errno == ETIMEDOUT)) {
//		printf("TIMEOUT\n");
		return -1;
	}

	return 0;
}

void cmd_resp(void)
{
	sem_post(&cmd_sem);
}

int cmd_init(void)
{
	sem_init(&cmd_sem, 0, 0);

	return 0;
}
