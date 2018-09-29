#ifndef __PANTHER_DEV_H__
#define __PANTHER_DEV_H__

#include <os_compat.h>

struct panther_dev {
	/* RF device */
	struct{
		u8 chip_ver;
		u8 power_level; /* RF power value */
		u8 if_type;
	}rf;
	u32 bb_reg_tbl;
	u32 usec;
	u32 usec_overflow;

	u8 static_buf[20]; /* for static buffer of inet_ntoa(), ether_ntoa(), and ether_aton() */
};
extern struct panther_dev *ldev;
#endif
