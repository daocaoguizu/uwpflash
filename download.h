#ifndef _DOWNLOAD_H_


extern int dl_init(void);
extern int dl_flash_fdl(unsigned char *fdl, unsigned int len, unsigned int addr);
extern int dl_flash(char *fname, unsigned int addr);

#endif
