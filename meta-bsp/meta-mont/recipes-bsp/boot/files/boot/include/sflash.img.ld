/*
	0       -  0x10000  boot.img
	0x10000 -  0x1ffff  parameter
	0x20000 ~           app.img
*/

SECTIONS
{
	BOOTLDR 0x0 :
	{
		boot*
		. = 0x20000 ;
	} = 0xffffffff
}
