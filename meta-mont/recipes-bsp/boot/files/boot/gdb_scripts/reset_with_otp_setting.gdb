file boot1_debrick
shell sleep 1
target remote localhost:3333
mon halt
restore boot1_debrick
mon reg pc 0x90000000
mon resume
shell sleep 1
mon halt
#you modify the OTP setting in the line below
set *(unsigned long *)0xbf0048fc=0x0000011B
set *(unsigned long *)0xbf004814=0x01000001
set *(unsigned long *)0xbf004804=0x00000081
set confirm off
quit
