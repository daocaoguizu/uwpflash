#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#define FILE_IN  "fdl.bin"
#define FILE_OUT "fdl.h"

static int fd_in;
static int fd_out;
char str_header[] = "#ifndef _FDL_H__\nchar fdl[] = {\n";
char str_tail[] = "};\n#endif\n";

int main(int argc, char **argv)
{
	int i;
	int ret;

	fd_in = open(FILE_IN, O_RDONLY);
	if (fd_in < 0) {
		perror("file");
		return -1;
	}


	fd_out = open(FILE_OUT, O_WRONLY);
	if (fd_out < 0) {
		perror("file");
		return -1;
	}

	write(fd_out, str_header, strlen(str_header));

	unsigned char c;
	unsigned int len;
	i = 0;
	char buf[128];
	while (1) {
		len = read(fd_in, &c, 1);

		if (len != 1) {
			printf("finish.\n");
			break;
		}

		sprintf(buf, "0x%02x, ", c);
		
		write(fd_out, buf, 6);
		i++;

		if (i % 16 == 0) {
			write(fd_out, "\n", 1);
		}
	}
	write(fd_out, str_tail, strlen(str_tail));

	close(fd_out);
}
