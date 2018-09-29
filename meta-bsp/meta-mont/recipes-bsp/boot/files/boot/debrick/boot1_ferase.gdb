#target remote localhost:3333
#mon halt
#set *(unsigned long *)0xbf004820=0x01000001
#mon resume
#disconnect
file boot1_ferase
shell sleep 2
target remote localhost:3333
mon halt
restore boot1_ferase
#mon reg pc 0xB0000000
mon reg pc 0x90000000
mon resume
set confirm off
quit
