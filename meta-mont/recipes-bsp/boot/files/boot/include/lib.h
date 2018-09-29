#ifndef __LIB_H__
#define __LIB_H__

#define UNUSED(x) (void)(x)

int sscanf(char *buf, char *fmt, ...);
int sprintf(char *buf, char *fmt, ...);
int strcmp(const char *dst, const char *src);
void memcpy(void *dst, void *src, int len);
int atoi(char *s);
void append_chr(char *str, char c);
void ins_chr(char *str, char c);
char *strcpy(char *dst, const char *src);
void strncpy(void *dst, const void *src, int len);
void memset(char *p, int data, int len);
void puts(char *s);
void putc(char c);
int hextoul(char *str, void *v);
int otp_parse_config(int shift_val);
void serial_init(void);
int serial_poll(void);
int serial_getc(void);
void serial_putc(int c);
int memcmp(void *dst, void *src, int len);
void memmove(void *dst, void *src, int len);
void dump_hex(char *msg, char *buf, int len);
int strlen(const char *str);
int strncmp(const void *dst, const void *src, int len);
char *strcat(char *s1, const char *s2);
void dump_hex4(char *msg, unsigned int *buf, int len);
int gets(char *buf);
int getchar(void);
int putchar(int cc);
int strtol(char *s, char **rp, int base);
int strcasecmp(char *a, char *b);
int inet_aton(char *str, void *dp);
char *inet_ntoa(void *ipp);
int vaddr_check(unsigned long addr);
unsigned short ipchksum(void *data, int len);
int rand(void);
void srf_exit(unsigned long global_ddr_size);
void srf_enter(void);
void cmd_loop(void);
char *boot_mode_str(int mode);
void cmd_init();
int cmd_go(int argc, char *argv[]);
int cmd_cmp(int argc, char *argv[]);
int do_usb(int argc, char *argv[]);
int tstc(void);
int flash_cmd(int argc, char *argv[]);
int eth_open(int intf);
int ether_aton(char *buf, unsigned char *mac);
char *ether_ntoa(unsigned char *m);
void tsi_init(void);
int cdb_init(int id);
int cdb_save(int flag);
void init_cdb_page_idx(void);
void config_init(int *s);
int gpio_swrst_check(void);
int uart_loopback_check(void);
int cmd_rst(int argc, char *argv[]);
void write_be32(unsigned char *a, unsigned int v);
unsigned int read_le32(const unsigned char *a);
void write_le16(unsigned char *a, unsigned short v);
void otp_program_nor_4addr_mode(void);

#endif //__LIB_H__
