target remote localhost:3333
mon halt
set *(unsigned long *)0xbf004820=0x01000001
mon resume
disconnect
set confirm off
quit
