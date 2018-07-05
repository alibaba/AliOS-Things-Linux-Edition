#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#define	TIOCGRS485	0x542E
#define	TIOCSRS485	0x542F

static char note[] = {
	"\nNote: "
	"Please use this tool after opening UART and setting the baud rate.\n"
	"\nOn the command you need to include one parameter: \n"
	"	0 : Turn off RS485.\n"
	"	1 : Turn on RS485.\n"
	"\nExample command line:\n"
	"	imx_rs485 0\n"
	"	imx_rs485 1\n\n"
};

int main(int argc, char *argv[])
{
	int fd;
	struct serial_rs485 rs485conf;

	if (argc != 2) {
		
		printf(note);
		return 0;
	}

	fd = open("/dev/ttymxc0", O_RDWR);
	if (fd < 0) {
		perror("Open ttymxc0");
		return -1;
	}

	if (ioctl(fd, TIOCGRS485, &rs485conf) < 0) {
		printf("ioctl GET RS485 FAIL\n");
	}

//		rs485conf.flags |= SER_RS485_ENABLED;
	if (argc == 2) {
		if (argv[1][0] == '1') {
			printf("Turn on RS485\n");
			rs485conf.flags |= SER_RS485_ENABLED;
		} else if (argv[1][0] == '0') {
			printf("Turn off RS485\n");
			rs485conf.flags &= ~SER_RS485_ENABLED;
		} else {
			printf("imx_rs485 0 : Turn off RS485\nimx_rs485 1 : Turn on RS485\n");
		} 
	}

	
	if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) {
		/* Error handling. See errno. */
		printf("ioctl SET RS485 FAIL\n");
	}
	
	/* Close the device when finished: */
	if (close(fd) < 0) {
		/* Error handling. See errno. */
		perror("Close ttymxc0");
	}

	return 0;
}
