#include "image.h"

#ifndef IH_LOAD
#define IH_LOAD 0
#endif

struct img_head ih =
{
magic: 	{ 'A','I','H' },
hlen: 	IH_HLEN,
time: 	IH_TIME ,
run: 	IH_RUN ,
size: 	IH_SIZE ,
load_addr: IH_LOAD,
mid:	IH_MID,
ver:	IH_VER,
chksum:	IH_CHKSUM,
desc: 	"ip3210 linux"
};
