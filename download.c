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

#include "command.h"
#include "interface.h"

#define MAX_DATA_LEN 1024
#define RECV_BUF_LEN 1024
#define MAX_PERCENT	(50)
#define CHECK_BANDRATE_TIME_SEC	(10)

struct dl_file
{
	int fd;
	int is_fdl;
	unsigned char *fdl_buf;
	unsigned int addr;
	unsigned int pos;
	unsigned int len;
	unsigned int print_percent;
};

static unsigned char *data_buf;
static struct dl_file dl_file;

pthread_t tid_recv;

void *dl_recv_thread(void *arg)
{
	int len;
	char buf[RECV_BUF_LEN];
	struct packet_header *pkt_hdr;
	
	while (1) {
		len = RECV_BUF_LEN;
		len = intf_recv(buf, len, 5);
		if (len > 0) {
			pkt_hdr = (struct packet_header *)buf;
			pkt_hdr->type = le16(pkt_hdr->type);
			pkt_hdr->size = le16(pkt_hdr->size);

			//printf("pkt type: 0x%02x.\n", pkt_hdr->type);
			//printf("pkt size: %d.\n", pkt_hdr->size);

			if ((pkt_hdr->type == BSL_REP_VER) ||
				(pkt_hdr->type == BSL_REP_ACK)){
				cmd_resp();
			} else {
				printf("Unknown header type: 0x%x.\n",
						pkt_hdr->type);
			}
		}
	}
}

int dl_send_data(void)
{
	int i;
	int ret;
	struct dl_file *pfile = &dl_file;
	unsigned int data_len;
	unsigned int dl_percent;

	for (i = 0; i < MAX_PERCENT; i++) {
		printf("-");
	}
	fflush(stdout);
	for (i = 0; i < MAX_PERCENT; i++) {
		printf("\b");
	}

	while (1) {
		data_len = pfile->len - pfile->pos;

		if (data_len == 0)
			break;

		if (data_len > MAX_DATA_LEN)
			data_len = MAX_DATA_LEN;

		if (pfile->is_fdl) {
			memcpy(data_buf, pfile->fdl_buf + pfile->pos, data_len);
		} else {
			lseek(pfile->fd, pfile->pos, SEEK_SET);

			data_len = read(pfile->fd, data_buf, data_len);
		}

		pfile->pos += data_len;

		dl_percent = pfile->pos * MAX_PERCENT / pfile->len;
		for (i = pfile->print_percent; i < dl_percent; i++) {
			printf(">");
			fflush(stdout);
		}
		pfile->print_percent = dl_percent;

		ret = cmd_send_data(data_buf, data_len);
		if (ret < 0) {
			printf("\n");
			return ret;
		}
	}

	return 0;
}

int dl_check_bandrate(void)
{
	int i;
	printf("* CHECKING BANDRATE...  \t");
	fflush(stdout);

	for (i = 0; i < CHECK_BANDRATE_TIME_SEC; i++) {
		if (cmd_check_bandrate(1) == 0) {
			printf("Done\n");
			fflush(stdout);
			return 0;
		}
	}

	return -1;
}

int dl_flash(int is_fdl)
{
	int ret;

	printf("* CONNECTING...\t\t\t");
	fflush(stdout);
	ret = cmd_connect(5);
	if (ret < 0) return ret;
	printf("Done\n");
	fflush(stdout);

	printf("* START...\t\t\t");
	fflush(stdout);
	ret = cmd_send_start(dl_file.addr, dl_file.len);
	if (ret < 0) return ret;
	printf("Done\n");
	fflush(stdout);

	ret = dl_send_data();
	if (ret < 0) return ret;

	printf("\n* STOP...\t\t\t");
	fflush(stdout);
	ret = cmd_stop(20);
	if (ret < 0) return ret;
	printf("Done\n");
	fflush(stdout);

	if (is_fdl) {
		printf("* RUN...\t\t\t");
		fflush(stdout);
		ret = cmd_exec(10);
		if (ret < 0) return ret;
		printf("Done\n");
		fflush(stdout);
	}

	printf("* SUCCESS\n\n");

	return 0;
}

int dl_flash_fdl(unsigned char *fdl, unsigned int len, unsigned int addr)
{
	int ret;
	struct dl_file *pfile = &dl_file;

	memset((void*)pfile, 0, sizeof(struct dl_file));
	pfile->is_fdl = 1;
	pfile->addr = addr;
	pfile->len = len;
	pfile->fdl_buf = fdl;

	printf("download FDL (%d bytes)\n", pfile->len);

	ret = dl_check_bandrate();
	if (ret < 0) return ret;

	return dl_flash(1);
}

int dl_flash_file(char *fname, unsigned int addr)
{
	int ret;
	struct dl_file *pfile = &dl_file;
	memset((void*)pfile, 0, sizeof(struct dl_file));

	pfile->addr = addr;
	pfile->fd = open(fname, O_RDONLY);
	if (pfile->fd < 0) {
		perror(fname);
		return -1;
	}

	if ((pfile->len = lseek(pfile->fd, 0, SEEK_END)) < 0)
	{
		perror("lseek file failure!");
		return -1;
	}
	lseek(pfile->fd, 0, SEEK_SET);

	printf("download file %s (%d bytes)\n", fname, pfile->len);

	
	return dl_flash(0);
}

int dl_init(void)
{
	int ret;

	data_buf = malloc(MAX_DATA_LEN);
	if (data_buf == NULL) {
		printf("out of memory.\n");
		return -1;
	}

	ret = pthread_create(&tid_recv, NULL, dl_recv_thread, NULL);
	if (ret) {
		perror("create pthread failed");
		return ret;
	}

	return 0;
}
