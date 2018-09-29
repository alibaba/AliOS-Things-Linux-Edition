/*
	0       -  0x1fff  boot1
	0x2000  -  0x5fff  parameter
	0x6000  -  0xffff  boot2
*/

SECTIONS
{
	BOOT1 0x0 :
	{
		boot1*
		. = 0x6000 ;
	} = 0xffffffff

	BOOT2 :
	{
		boot2*
		. = 0xa000 ;
	} = 0xffffffff
}
