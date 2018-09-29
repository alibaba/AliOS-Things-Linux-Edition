target remote localhost:3333
mon halt
set *(unsigned long *)0xbf005520=0x0
mon reg pc 0xbfc00000
set *(unsigned long *)0xbf004820=0x01000001
mon resume
disconnect
file boot2
shell sleep 2
target remote localhost:3333
mon halt
load
set *(unsigned long *)0xbf005520=0x82000000
mon resume
set confirm off
quit
