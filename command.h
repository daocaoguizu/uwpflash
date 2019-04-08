#ifndef _COMMAND_H_

#define FLAG_BYTE               0x7E
enum CMD_TYPE
{
	BSL_PKT_TYPE_MIN, /* the bottom of the DL packet type range */

	/* Link Control */
	BSL_CMD_CONNECT = BSL_PKT_TYPE_MIN,   /* 0x00

	/* Data Download */
	BSL_CMD_START_DATA,   /* 0x01 the start flag of the data downloading  */
	BSL_CMD_MIDST_DATA,   /* 0x02 the midst flag of the data downloading  */
	BSL_CMD_END_DATA, /* 0x03 the end flag of the data downloading */
	BSL_CMD_EXEC_DATA,    /* 0x04 Execute from a certain address */
	/* End of Data Download command */

	BSL_CMD_CHECK_BAUD = FLAG_BYTE,   /* CheckBaud command,for internal use */
	BSL_CMD_END_PROCESS = 0x7F,   /* 0x7F End flash process */
	/* End of the Command can be received by phone */

	/* Start of the Command can be transmited by phone */
	BSL_REP_TYPE_MIN = 0x80,

	BSL_REP_ACK = BSL_REP_TYPE_MIN,   /* The operation acknowledge */
	BSL_REP_VER      /* 0x81 */
};

struct packet_header
{
	unsigned short type;
	unsigned short size;
};

static inline unsigned int le32(unsigned int num)
{
	return (((num >> 24) & 0xFF) | \
	((num >> 8) & 0xFF00) | \
	((num << 8) & 0xFF0000) | \
	((num << 24) & 0xFF000000));
}

#define le16(num) \
	(num >> 8) | (num << 8)

extern void cmd_resp(void);
extern int cmd_init(void);
extern int cmd_connect(int timeout);
extern int cmd_check_bandrate(int timeout);
extern int cmd_send_data(unsigned char * addr, unsigned int len);
extern int cmd_send_start(unsigned int addr, unsigned int len);
extern int cmd_exec(int timeout);
extern int cmd_stop(int timeout);

#endif
