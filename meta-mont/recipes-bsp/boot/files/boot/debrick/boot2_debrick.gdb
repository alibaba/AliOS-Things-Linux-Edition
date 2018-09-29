file boot2
target remote localhost:3333
mon halt
restore boot2
set *(unsigned long *)0xbf005520=0x83800000
mon resume
set confirm off
quit
