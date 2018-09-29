file boot3
target remote localhost:3333
mon halt
restore boot3
mon reg pc 0x83000000
c
